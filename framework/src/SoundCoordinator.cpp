#include "SoundCoordinator.h"
#include "Utility.h"

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

unsigned int SoundCoordinator::getSystemSamplingRate()
{
    return mSystemSamplingRate;
}

unsigned int SoundCoordinator::getSystemChannels()
{
    return mSystemChannels;
}

RARetCode SoundCoordinator::registerRenderer(ISoundRenderer* renderer)
{
    mSoundData.push_back(renderer);
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

RARetCode SoundCoordinator::DataWriter::write(float* buff, unsigned int frameLen)
{
    if (!buff || mWriteBufferSize < (mWroteFrame + frameLen) * SC_CHANNEL ) {
        return RARetCode::RET_ERR_INVALID_ARG;
    }

    if (!mWriteBuffer) {
        return RARetCode::RET_ERR_INVALID_STATE;
    }

    for (unsigned int frame = mWroteFrame; frame < mWroteFrame + frameLen; ++frame) {
        
        for (unsigned int ch = 0; ch < SC_CHANNEL; ch++) {
            *(mWriteBuffer + frame * SC_CHANNEL + ch) += *(buff + frame * SC_CHANNEL + ch);
        }
    }
    mWroteFrame += frameLen;
    return RARetCode::RET_OK;
}

void SoundCoordinator::DataWriter::setBuffer(float* buff, unsigned int size)
{
    mWriteBuffer     = buff;
    mWriteBufferSize = size;
}

void SoundCoordinator::DataWriter::reset()
{
    mWroteFrame = 0;
}


SoundCoordinator::DataWriter::DataWriter() :
      mWriteBuffer(nullptr)
    , mWriteBufferSize(0)
    , mWroteFrame(0)

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
            unsigned int frame = 0;
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

unsigned int SoundCoordinator::requestDataInternal(float** buff, unsigned int frameNum)
{
    unsigned int outFrameNum = 0;
    float*       outBuff     = *buff;
    unsigned int count       = 0;

    while (outFrameNum < frameNum) {
        ////////////////////////////////////////////////////////////
        // Process for writing to SC Buffer
        ////////////////////////////////////////////////////////////
        SoundBuffer& writeSB = mSystemBuffer.mBuffer[mSystemBuffer.mWritePointer];

        if (!mSoundData.empty()) {
            float* writePoint = &writeSB.mBuffer[writeSB.mWritePointer];
            std::fill(writePoint, writePoint + (writeSB.mBufferSize - writeSB.mWritePointer), 0.0f);

            mDataWriter.setBuffer(writePoint, writeSB.mBufferSize - writeSB.mWritePointer);

            unsigned int retFrameMax = 0;

            for (auto ite = mSoundData.begin(); ite != mSoundData.end(); )
            {
                ISoundRenderer* sound = *ite;
                unsigned int    retFrameLen     = 0;
                unsigned int    requestFrameLen = 0;
                RARetCode       ret;

                mDataWriter.reset();
                requestFrameLen = (writeSB.mBufferSize - writeSB.mWritePointer) / SC_CHANNEL;
                ret = sound->requestData(requestFrameLen, &retFrameLen, mDataWriter);

                if (retFrameMax < retFrameLen) {
                    retFrameMax = retFrameLen;
                }

                if (ret == RARetCode::RET_END_OF_CONTENT) {
                    ite = mSoundData.erase(ite);
                }
                else {
                    ++ite;
                }
            }
//            unsigned int ret = requestData(&writePoint, (writeSB.mBufferSize - writeSB.mWritePointer) / SC_CHANNEL);
            writeSB.mWritePointer += retFrameMax * SC_CHANNEL;
        }

        count = 0;
        for (unsigned int writePoint = writeSB.mWritePointer; writePoint < writeSB.mBufferSize; writePoint += SC_CHANNEL) {
            for (unsigned int ch = 0; ch < SC_CHANNEL; ++ch) {
                writeSB.mBuffer[writePoint + ch] = 0.0f;
                ++count;
            }
        }
        writeSB.mWritePointer += count;

        if (writeSB.mBufferSize <= writeSB.mWritePointer) {
            mSystemBuffer.mWritePointer = (mSystemBuffer.mWritePointer + 1) & 0x1;
        }

        ////////////////////////////////////////////////////////////
        // Process for writing to System Buffer
        ////////////////////////////////////////////////////////////

        if (mSystemSamplingRate == SC_SAMPLING_RATE) {
            SoundBuffer& readSB = mSystemBuffer.mBuffer[mSystemBuffer.mReadPointer];

            count = 0;
            for (unsigned int readPoint = readSB.mReadPointer
                ; readPoint < readSB.mWritePointer && outFrameNum < frameNum
                ; readPoint += mSystemChannels) {

                unsigned int buffCh = 0;
                for (unsigned int sysCh = 0; sysCh < mSystemChannels; ++sysCh) {
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
                readSB.mReadPointer = 0;
                readSB.mWritePointer = 0;
                mSystemBuffer.mReadPointer = (mSystemBuffer.mReadPointer + 1) & 0x1;
            }

        } else { //mSystemSamplingRate != SC_SAMPLING_RATE
            SoundBuffer& readSB = mSystemBuffer.mBuffer[mSystemBuffer.mReadPointer];

            unsigned int srcInDataLen = readSB.mWritePointer - readSB.mReadPointer;

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
            }

            for (unsigned int readPoint = mSystemSRCOutFrameCurrent * SC_CHANNEL
                ; readPoint < mSystemSRCOutFrameLen && outFrameNum < frameNum
                ; readPoint += mSystemChannels) {

                unsigned int buffCh = 0;
                for (unsigned int sysCh = 0; sysCh < mSystemChannels; ++sysCh) {
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

void SoundCoordinator::makeChannelOffsetMap(unsigned int mask)
{
    unsigned int frontL  = 0x00000001;
    unsigned int frontR  = 0x00000002;
    unsigned int center  = 0x00000004;
    unsigned int lowFreq = 0x00000008;
    unsigned int backL   = 0x00000010;
    unsigned int backR   = 0x00000020;
    unsigned int sideL   = 0x00000200;
    unsigned int sideR   = 0x00000400;

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
        unsigned int pattern[8] = { frontL , frontR , center , lowFreq , backL , backR , sideL , sideR };
        unsigned int ch     = 0;
        for (unsigned int i = 0; i < 8; i++) {
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

