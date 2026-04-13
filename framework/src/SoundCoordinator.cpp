#include "SoundCoordinator.h"
#include "Utility.h"

#include <cstdint>
#include <memory>
#include <mmdeviceapi.h>
#include <combaseapi.h>
#include <audioclient.h>
#include <fstream>
#include <iostream>
#include <mutex>
#include <atlcomcli.h>

#pragma comment(lib, "ole32.lib")

////////////////////////////////////
// APIs
////////////////////////////////////
RARetCode SoundCoordinator::start()
{
    if (mStarted) {
        return RARetCode::RET_ERR_INVALID_STATE;
    }

    mStarted = true;

    mThread = std::thread(&SoundCoordinator::renderToDevice, this);
    return RARetCode::RET_OK;
}

RARetCode SoundCoordinator::stop()
{
    if (!mStarted) {
        return RARetCode::RET_ERR_INVALID_STATE;
    }

    mStarted = false;

    if (mThread.joinable()) {
        mThread.join();
    }
    return RARetCode::RET_OK;
}

uint32_t SoundCoordinator::getSystemSamplingRate()
{
    return mSystemSamplingRate;
}

uint32_t SoundCoordinator::getSystemChannels()
{
    return mSystemChannels;
}

uint32_t SoundCoordinator::getDelayTime()
{
    return static_cast<uint32_t>(mAverageDelayTime) + (SYSTEM_BUFFER_DURATION/10);
}

RARetCode SoundCoordinator::registerRenderer(ISoundRenderer* renderer)
{
    mMutex.lock();
    mRenderers.push_back(renderer);
    mMutex.unlock();
    return RARetCode::RET_OK;
}

RARetCode SoundCoordinator::unregisterRenderer(ISoundRenderer* renderer)
{
    RARetCode ret = RARetCode::RET_OK;

    if (!renderer) {
        return RARetCode::RET_ERR_INVALID_ARG;
    }

    mMutex.lock();
    if (0 != Utility::eraseVectorElm(mRenderers, renderer)) {
        ret = RARetCode::RET_ERR_INVALID_ARG;
    }
    mMutex.unlock();
    return ret;
}


void SoundCoordinator::recover()
{
    Utility::printLog("Start Recovering");
    mRecover = true;
}

SoundCoordinator::SoundCoordinator() :
      mStarted(false)
    , mRecover(false)
    , mSystemChannels(2)
    , mSystemSamplingRate(48000)
    , mChOffsetMap{0,1}
    , mSCBuffer(std::make_unique<MultiBlockBufferInternal>(
                                      SC_BUFFER_BLOCK_NUM
                                    , SC_BUFFER_BASE_SIZE
                                    , SC_CHANNEL))
    , mSCSRCOutBuff(nullptr)
    , mSCSRCOutFrameCurrent(0)
    , mSCSRCOutFrameLen(0)
    , mAverageDelayTime(0.0f)
{
};

////////////////////////////////////
// Private
////////////////////////////////////

RARetCode SoundCoordinator::DataWriter::write(float* buff, uint32_t frameLen)
{
    if (!buff || mWriteAvailFrames < mWroteFrame + frameLen) {
        return RARetCode::RET_ERR_INVALID_ARG;
    }

    if (!mWriteBuffer) {
        return RARetCode::RET_ERR_INVALID_STATE;
    }

    for (uint32_t frame = mWroteFrame; frame < mWroteFrame + frameLen; ++frame) {
        for (uint32_t ch = 0; ch < SC_CHANNEL; ch++) {
            *(mWriteBuffer + frame * SC_CHANNEL + ch) += *(buff + frame * SC_CHANNEL + ch);
        }
    }
    
    mWroteFrame += frameLen;

    return RARetCode::RET_OK;
}

void SoundCoordinator::DataWriter::setBuffer(float*& buffer, uint32_t availFrames)
{
    mWriteBuffer      = buffer;
    mWriteAvailFrames = availFrames;
}


void SoundCoordinator::DataWriter::reset()
{
    mWroteFrame = 0;
}


SoundCoordinator::DataWriter::DataWriter() :
      mWroteFrame(0)
    , mWriteAvailFrames(0)
    , mWriteBuffer(nullptr)
{
}


// Helper for COM API
#define CALL_WITH_RETURN(x,y)   if (S_OK != x){Utility::printLog(y);return;} 
#define CALL_WITH_CONTINUE(x,y) if (S_OK != x){Utility::printLog(y);continue;} 
#define CALL_WITH_BREAK(x,y)    if (S_OK != x){Utility::printLog(y);break;} 
#define CALL_WITH_THROUGH(x,y)  if (S_OK != x){Utility::printLog(y);} 

void SoundCoordinator::renderToDevice()
{
    Utility::printLog("Start Sound Rendering Thread");

    CALL_WITH_RETURN(CoInitializeEx(nullptr, COINIT_MULTITHREADED), "CoInitializeEx fails");

    while (mStarted) {
        Utility::printLog("Start initialization...");

        mRecover = false;

        CComPtr<IMMDeviceEnumerator> enumerator   = nullptr;
        CComPtr<IMMDevice>           device       = nullptr;
        CComPtr<IAudioClient>        audioClient  = nullptr;
        CComPtr<IAudioRenderClient>  renderClient = nullptr;

        HANDLE audioEvent;

        std::unique_ptr<SystemNotification> systemNotification
            = std::make_unique<SystemNotification>(*this);

        UINT32                  buffFrameCount = 0;
        WAVEFORMATEX*           mixFormat      = nullptr;
        WAVEFORMATEXTENSIBLE*   mixFormatEx    = nullptr;
        REFERENCE_TIME          bufferDuration = SYSTEM_BUFFER_DURATION;

        CALL_WITH_CONTINUE(CoCreateInstance(
            __uuidof(MMDeviceEnumerator),nullptr,CLSCTX_ALL,IID_PPV_ARGS(&enumerator))
          , "CoCreateInstance for IMMDeviceEnumerator fails");
        

        CALL_WITH_CONTINUE(enumerator->GetDefaultAudioEndpoint(
            eRender, eConsole, &device), "GetDefaultAudioEndpoint fails");


        CALL_WITH_CONTINUE(enumerator->RegisterEndpointNotificationCallback(systemNotification.get())
            , "RegisterEndpointNotificationCallback fails");

        CALL_WITH_CONTINUE(device->Activate(
            __uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&audioClient)
            , "Device Activate fails");

        CALL_WITH_CONTINUE(audioClient->GetMixFormat(&mixFormat)
            , "GetMixFormat fails");

        mixFormatEx = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(mixFormat);

        mSystemChannels     = mixFormatEx->Format.nChannels;
        mSystemSamplingRate = mixFormat->nSamplesPerSec;
        makeChannelOffsetMap(mixFormatEx->dwChannelMask);

        Utility::printLog("Update System Audio Config:fs %d, ch %d", mSystemSamplingRate,mSystemChannels);

        if (SC_SAMPLING_RATE != mSystemSamplingRate) {
            mSCSrc.setConfig(SC_SAMPLING_RATE, SC_CHANNEL, mSystemSamplingRate);
        }

        audioEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (!audioEvent) {
            Utility::printLog("CreateEvent fails");
            continue;
        }

        CALL_WITH_CONTINUE(audioClient->Initialize(
            AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, bufferDuration
            , 0, mixFormat, nullptr), "Audio Client Initialize fails");

        CALL_WITH_CONTINUE(audioClient->SetEventHandle(audioEvent)
            , "SetEventHandle fails");

        CALL_WITH_CONTINUE(audioClient->GetBufferSize(&buffFrameCount)
            , "GetBufferSize fails");

        CALL_WITH_CONTINUE(audioClient->GetService(IID_PPV_ARGS(&renderClient))
            , "GetService fails");

        CALL_WITH_CONTINUE(audioClient->Start() 
            , "Audio Client Start fails");

        Utility::printLog("Finish initialization");
        while (mStarted && !mRecover)
        {
            UINT32 padding;
            UINT32 framesAvailable;
            BYTE*  data;
            float* out;

            WaitForSingleObject(audioEvent, INFINITE);

            CALL_WITH_BREAK(audioClient->GetCurrentPadding(&padding)
                , "GetCurrentPadding fails");

            framesAvailable = buffFrameCount - padding;
            if (framesAvailable == 0) {
                continue;
            }

            CALL_WITH_BREAK(renderClient->GetBuffer(framesAvailable, &data)
                ,"GetBuffer fails");

            out = (float*)data;
            uint32_t frame = 0;
            while (frame < framesAvailable) {
                frame = requestDataInternal(&out, framesAvailable);
                out += frame;
            }

            CALL_WITH_BREAK(renderClient->ReleaseBuffer(framesAvailable, 0)
                , "ReleaseBuffer fails");
        }

        Utility::printLog("Start termination...");
        CALL_WITH_THROUGH(audioClient->Stop()
            ,"Stop fails");

        CALL_WITH_THROUGH(audioClient->Reset()
            ,"Reset fails");

        CoTaskMemFree(mixFormat);

        CALL_WITH_THROUGH(enumerator->UnregisterEndpointNotificationCallback(systemNotification.get())
            ,"UnregisterEndpointNotificationCallback fails");

        Utility::printLog("Finish termination");
    }
    CoUninitialize();
    Utility::printLog("Stop Sound Rendering Thread");
}

uint32_t SoundCoordinator::requestDataInternal(float** buff, uint32_t frameNum)
{
    uint32_t outFrameNum = 0;
    float*   outBuff     = *buff;
    uint32_t count       = 0;
    uint64_t currentTime;

    while (outFrameNum < frameNum) {
        ////////////////////////////////////////////////////////////
        // Process for writing to SC Buffer
        ////////////////////////////////////////////////////////////
        float*   writeBuffer;
        uint32_t writeAvailFrameLen;

        mSCBuffer->getWriteBuffer(writeBuffer, writeAvailFrameLen);

        if (writeBuffer && writeAvailFrameLen > 0) {
            uint32_t updateSize = writeAvailFrameLen;
            std::fill(writeBuffer, writeBuffer + writeAvailFrameLen * SC_CHANNEL, 0.0f);
            if (!mRenderers.empty()) {

                mDataWriter.setBuffer(writeBuffer, writeAvailFrameLen);

                uint32_t retFrameMax = 0;

                mMutex.lock();
                for (auto ite = mRenderers.begin(); ite != mRenderers.end(); )
                {
                    ISoundRenderer* sound = *ite;

                    RARetCode       ret;
                    uint32_t        retFrameLen     = 0;

                    mDataWriter.reset();
                    ret = sound->requestData(writeAvailFrameLen, &retFrameLen, mDataWriter);

                    if (retFrameMax < retFrameLen) {
                        retFrameMax = retFrameLen;
                    }

                    if (ret == RARetCode::RET_END_OF_CONTENT) {
                        ite = mRenderers.erase(ite);
                    }
                    else {
                        ++ite;
                    }
                }
                updateSize = retFrameMax;
                mMutex.unlock();
            }
            currentTime = Utility::getCurrentTime();
            if (!mSCBuffer->updateWriteBuffer(updateSize, &currentTime)) {
                Utility::printLog("updateWriteBuffer failed");
            }
        }

        ////////////////////////////////////////////////////////////
        // Process for writing to System Buffer
        ////////////////////////////////////////////////////////////
        float*   readBuffer;
        uint32_t readAvailFrameLen;
        uint32_t readFrameCount;

        if (mSystemSamplingRate == SC_SAMPLING_RATE) {

            mSCBuffer->getReadBuffer(readBuffer, readAvailFrameLen);

            if (readBuffer && readAvailFrameLen > 0) {

                readFrameCount = 0;
                for (uint32_t i = 0; i < readAvailFrameLen && outFrameNum < frameNum; ++i) {
                    uint32_t scCh = 0;
                    for (uint32_t sysCh = 0; sysCh < mSystemChannels; ++sysCh) {
                        if (mChOffsetMap[scCh] == sysCh) {
                            *outBuff++ = *(readBuffer + i * SC_CHANNEL + scCh);
                            ++scCh;
                        }
                        else {
                            *outBuff++ = 0.0f;
                        }
                    }
                    ++readFrameCount;
                    ++outFrameNum;
                }
                uint64_t timeFromBuffer;
                if (!mSCBuffer->updateReadBuffer(readFrameCount, &timeFromBuffer)) {
                    Utility::printLog("updateReadBuffer failed");
                }
                calcAverageDelayTime(Utility::getCurrentTime() - timeFromBuffer);

            }
        } else { //mSystemSamplingRate != SC_SAMPLING_RATE

            mSCBuffer->getReadBuffer(readBuffer, readAvailFrameLen);

            if (readBuffer && readAvailFrameLen > 0) {
                if (!mSCSRCOutBuff) {
                    mSCSrc.apply(readBuffer
                        , readAvailFrameLen
                        , &mSCSRCOutBuff
                        , &mSCSRCOutFrameLen);
                }
                mSCBuffer->updateReadBuffer(readAvailFrameLen,nullptr);

                readFrameCount = 0;
                for (uint32_t i = mSCSRCOutFrameCurrent * SC_CHANNEL
                    ; i < mSCSRCOutFrameLen && outFrameNum < frameNum
                    ; i += mSystemChannels) {

                    uint32_t scCh = 0;

                    for (uint32_t sysCh = 0; sysCh < mSystemChannels; ++sysCh) {
                        if (mChOffsetMap[scCh] == sysCh) {
                            *outBuff++ = mSCSRCOutBuff[i + scCh];
                            ++scCh;
                        }
                        else {
                            *outBuff++ = 0.0f;
                        }
                    }
                    ++outFrameNum;
                    ++readFrameCount;
                }
                mSCSRCOutFrameCurrent += readFrameCount;

                if (mSCSRCOutFrameLen <= mSCSRCOutFrameCurrent) {
                    mSCSrc.releaseBuffer();
                    mSCSRCOutBuff = nullptr;
                    mSCSRCOutFrameLen     = 0;
                    mSCSRCOutFrameCurrent = 0;
                }
            }
        }
    }
    return outFrameNum;
}

void SoundCoordinator::makeChannelOffsetMap(uint32_t mask)
{
    uint32_t frontL  = 0x00000001;
    uint32_t frontR  = 0x00000002;
    uint32_t center  = 0x00000004;
    uint32_t lowFreq = 0x00000008;
    uint32_t backL   = 0x00000010;
    uint32_t backR   = 0x00000020;
    uint32_t sideL   = 0x00000200;
    uint32_t sideR   = 0x00000400;

    mChOffsetMap[0] = 0;
    mChOffsetMap[1] = 0;

    if ((mask & frontL) && (mask & frontR)) {
        mChOffsetMap[0] = 0;
        mChOffsetMap[1] = 1;
    } else if (mask & center) {
        mChOffsetMap[0] = 0;
        mChOffsetMap[1] = 0;
    } else if ((mask & backL) && (mask & backR)) {
        mChOffsetMap[0] = 4;
        mChOffsetMap[1] = 5;
    } else {
        uint32_t pattern[8] = { frontL , frontR , center , lowFreq , backL , backR , sideL , sideR };
        uint32_t ch     = 0;
        for (uint32_t i = 0; i < 8; i++) {
            if (mask & pattern[i]) {
                mChOffsetMap[ch] = i;
                ch++;
                if (ch == 2) {
                    break;
                }
            }
        }
    }
}

void SoundCoordinator::calcAverageDelayTime(uint64_t sample)
{
    mAverageDelayTime += ((float)sample - mAverageDelayTime) * 0.3f;
}

