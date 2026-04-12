#include "SoundCoordinator.h"
#include "Utility.h"

#include <cstdint>
#include <memory>
#include <mmdeviceapi.h>
#include <combaseapi.h>
#include <audioclient.h>
#include <fstream>
#include <iostream>
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
    mRenderers.push_back(renderer);
    return RARetCode::RET_OK;
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
    , mSystemSRCOutBuff(nullptr)
    , mSystemSRCOutFrameCurrent(0)
    , mSystemSRCOutFrameLen(0)
    , mAverageDelayTime(0.0f)
{
    allocateSystemBuffer();
};

SoundCoordinator::~SoundCoordinator()
{
    releaseSystemBuffer();
}


////////////////////////////////////
// Private
////////////////////////////////////

RARetCode SoundCoordinator::DataWriter::write(float* buff, uint32_t frameLen)
{
    float*   writeBuffer     = &mSoundBuffer->mBuffer[mSoundBuffer->mWritePointer];
    uint32_t writeBufferSize = mSoundBuffer->mBufferSize - mSoundBuffer->mWritePointer;

    if (!buff || writeBufferSize < (mWroteFrame + frameLen) * SC_CHANNEL ) {
        return RARetCode::RET_ERR_INVALID_ARG;
    }

    if (!writeBuffer) {
        return RARetCode::RET_ERR_INVALID_STATE;
    }

    for (uint32_t frame = mWroteFrame; frame < mWroteFrame + frameLen; ++frame) {
        for (uint32_t ch = 0; ch < SC_CHANNEL; ch++) {
            *(writeBuffer + frame * SC_CHANNEL + ch) += *(buff + frame * SC_CHANNEL + ch);
        }
    }
    mWroteFrame += frameLen;

    return RARetCode::RET_OK;
}

void SoundCoordinator::DataWriter::setBuffer(SoundBuffer* soundBuffer)
{
    mSoundBuffer = soundBuffer;
}


void SoundCoordinator::DataWriter::reset()
{
    mWroteFrame = 0;
}


SoundCoordinator::DataWriter::DataWriter() :
      mWroteFrame(0)
    , mSoundBuffer(nullptr)
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
            mSystemSrc.setConfig(SC_SAMPLING_RATE, SC_CHANNEL, mSystemSamplingRate);
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
    float*       outBuff     = *buff;
    uint32_t count       = 0;

    while (outFrameNum < frameNum) {
        ////////////////////////////////////////////////////////////
        // Process for writing to SC Buffer
        ////////////////////////////////////////////////////////////
        SoundBuffer& writeSB = mSystemBuffer.mBuffer[mSystemBuffer.mWritePointer];

        if (!mRenderers.empty()) {
            float* writePoint = &writeSB.mBuffer[writeSB.mWritePointer];
            std::fill(writePoint, writePoint + (writeSB.mBufferSize - writeSB.mWritePointer), 0.0f);

            mDataWriter.setBuffer(&writeSB);

            uint32_t retFrameMax = 0;

            for (auto ite = mRenderers.begin(); ite != mRenderers.end(); )
            {
                RARetCode       ret;
                ISoundRenderer* sound           = *ite;
                uint32_t        retFrameLen     = 0;
                uint32_t        requestFrameLen = 0;

                mDataWriter.reset();
                requestFrameLen = (writeSB.mBufferSize - writeSB.mWritePointer) / SC_CHANNEL;
                ret = sound->requestData(requestFrameLen, &retFrameLen, mDataWriter);

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
            writeSB.mWritePointer += retFrameMax * SC_CHANNEL;
        }

        count = 0;
        for (uint32_t writePoint = writeSB.mWritePointer; writePoint < writeSB.mBufferSize; writePoint += SC_CHANNEL) {
            for (uint32_t ch = 0; ch < SC_CHANNEL; ++ch) {
                writeSB.mBuffer[writePoint + ch] = 0.0f;
                ++count;
            }
        }
        writeSB.mWritePointer += count;
        writeSB.mWriteTime = Utility::getCurrentTime();

        if (writeSB.mBufferSize <= writeSB.mWritePointer) {
            mSystemBuffer.mWritePointer = (mSystemBuffer.mWritePointer + 1) & 0x1;
        }

        ////////////////////////////////////////////////////////////
        // Process for writing to System Buffer
        ////////////////////////////////////////////////////////////

        if (mSystemSamplingRate == SC_SAMPLING_RATE) {
            SoundBuffer& readSB = mSystemBuffer.mBuffer[mSystemBuffer.mReadPointer];

            count = 0;
            for (uint32_t readPoint = readSB.mReadPointer
                ; readPoint < readSB.mWritePointer && outFrameNum < frameNum
                ; readPoint += mSystemChannels) {

                uint32_t buffCh = 0;
                for (uint32_t sysCh = 0; sysCh < mSystemChannels; ++sysCh) {
                    if (mChOffsetMap[buffCh] == sysCh) {
                        *outBuff++ = readSB.mBuffer[readPoint + buffCh];
                        ++buffCh;
                        ++count;
                    }
                    else {
                        *outBuff++ = 0.0f;
                    }
                }
                ++outFrameNum;
            }
            readSB.mReadPointer += count;

            if (readSB.mBufferSize <= readSB.mReadPointer) {
                readSB.mReadPointer  = 0;
                readSB.mWritePointer = 0;
                mSystemBuffer.mReadPointer = (mSystemBuffer.mReadPointer + 1) & 0x1;
                calcAverageDelayTime(Utility::getCurrentTime() - readSB.mWriteTime);
            }

        } else { //mSystemSamplingRate != SC_SAMPLING_RATE
            SoundBuffer& readSB = mSystemBuffer.mBuffer[mSystemBuffer.mReadPointer];

            uint32_t srcInDataLen = readSB.mWritePointer - readSB.mReadPointer;

            if (!mSystemSRCOutBuff) {
                mSystemSrc.apply(&readSB.mBuffer[readSB.mReadPointer]
                    , srcInDataLen / SC_CHANNEL
                    , &mSystemSRCOutBuff
                    , &mSystemSRCOutFrameLen);
            }
            readSB.mReadPointer += srcInDataLen;

            if (readSB.mBufferSize <= readSB.mReadPointer) {
                readSB.mReadPointer  = 0;
                readSB.mWritePointer = 0;
                mSystemBuffer.mReadPointer = (mSystemBuffer.mReadPointer + 1) & 0x1;
                calcAverageDelayTime(Utility::getCurrentTime() - readSB.mWriteTime);
            }

            for (uint32_t readPoint = mSystemSRCOutFrameCurrent * SC_CHANNEL
                ; readPoint < mSystemSRCOutFrameLen && outFrameNum < frameNum
                ; readPoint += mSystemChannels) {

                uint32_t buffCh = 0;
                for (uint32_t sysCh = 0; sysCh < mSystemChannels; ++sysCh) {
                    if (mChOffsetMap[buffCh] == sysCh) {
                        *outBuff++ = mSystemSRCOutBuff[readPoint + buffCh];
                        ++buffCh;
                    }
                    else {
                        *outBuff++ = 0.0f;
                    }
                }
                ++outFrameNum;
                mSystemSRCOutFrameCurrent += outFrameNum;

                if (mSystemSRCOutFrameLen <= mSystemSRCOutFrameCurrent) {
                    mSystemSrc.releaseBuffer();
                    mSystemSRCOutBuff         = nullptr;
                    mSystemSRCOutFrameLen     = 0;
                    mSystemSRCOutFrameCurrent = 0;
                }
            }
        }
    }
    return outFrameNum;
}

void SoundCoordinator::allocateSystemBuffer()
{
    mSystemBuffer.mBuffer[0].mBuffer = new float[SYSTEM_BUFFER_BASE_SIZE * mSystemChannels];
    mSystemBuffer.mBuffer[1].mBuffer = new float[SYSTEM_BUFFER_BASE_SIZE * mSystemChannels];

    mSystemBuffer.mWritePointer = 0;
    mSystemBuffer.mReadPointer  = 0;

    mSystemBuffer.mBuffer[0].mBufferSize   = SYSTEM_BUFFER_BASE_SIZE * mSystemChannels;
    mSystemBuffer.mBuffer[0].mWritePointer = 0;
    mSystemBuffer.mBuffer[0].mReadPointer  = 0;

    mSystemBuffer.mBuffer[1].mBufferSize   = SYSTEM_BUFFER_BASE_SIZE * mSystemChannels;
    mSystemBuffer.mBuffer[1].mWritePointer = 0;
    mSystemBuffer.mBuffer[1].mReadPointer  = 0;

}

void SoundCoordinator::releaseSystemBuffer()
{
    delete[] mSystemBuffer.mBuffer[0].mBuffer;
    delete[] mSystemBuffer.mBuffer[1].mBuffer;
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



void SoundCoordinator::dumpSystemBufferCondition()
{
    Utility::printLog("===================");
    Utility::printLog("System Buffer:wp(%d) rp(%d)", mSystemBuffer.mWritePointer, mSystemBuffer.mReadPointer);
    Utility::printLog("Buffer1: wp(%d) rp(%d) size(%d)"
        , mSystemBuffer.mBuffer[0].mWritePointer, mSystemBuffer.mBuffer[0].mReadPointer, mSystemBuffer.mBuffer[0].mBufferSize);
    Utility::printLog("Buffer2: wp(%d) rp(%d) size(%d)"
        , mSystemBuffer.mBuffer[1].mWritePointer, mSystemBuffer.mBuffer[1].mReadPointer, mSystemBuffer.mBuffer[1].mBufferSize);
    Utility::printLog("===================");
}

