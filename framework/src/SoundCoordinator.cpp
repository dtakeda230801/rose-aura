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

unsigned int SoundCoordinator::getChannels()
{
    return mSystemChannels;
}

RARetCode SoundCoordinator::playOneShut(RequestDataFunction func)
{
    if (!func) {
        return RARetCode::RET_ERR_INVALID_ARG;
    }

    mRequestDataFuncs.push_back(func);
}


void SoundCoordinator::recover()
{
    Utility::printLog("Start Recovering");
    mRecover = true;
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

        UINT32          buffFrameCount = 0;
        WAVEFORMATEX*   mixFormat      = nullptr;
        REFERENCE_TIME  bufferDuration = SYSTEM_BUFFER_DURATION;

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

        mSystemChannels     = mixFormat->nChannels;
        mSystemSamplingRate = mixFormat->nSamplesPerSec;

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
            if (framesAvailable == 0)
            {
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

SoundCoordinator::SoundCoordinator() :
      mStarted(false)
    , mRecover(false)
    , mSystemChannels(2)
    , mSystemSamplingRate(48000)
    , mPlay(false)
    , mSRCOutBuff(nullptr)
    , mSRCOutFrameCurrent(0)
    , mSRCOutFrameLen(0)
    , mWaveFileHolder(nullptr)
{
    allocateSystemBuffer();

    mWaveFileHolder = new Utility::WaveFileHolder("M:\\e\\works\\Dev\\test.wav");
};

SoundCoordinator::~SoundCoordinator()
{
    releaseSystemBuffer();
    if (mWaveFileHolder) {
        delete mWaveFileHolder;
    }
}

void SoundCoordinator::test()
{
    if (!mPlay) {
        mPlay = true;

        mSRCOutBuff         = nullptr;
        mSRCOutFrameCurrent = 0;
        mSRCOutFrameLen     = 0;

        mSamplingRateConverter.reset();
        mSamplingRateConverter.setConfig(mWaveFileHolder->mSamplingRate
                                       , mSystemChannels
                                       , mSystemSamplingRate);
    }
}

unsigned int SoundCoordinator::requestData(float** buff, unsigned int frameLen)
{
    unsigned int ret        = 0;
    float*       renderBuff = *buff;

    float*       converterOut = nullptr;
    unsigned int converterOutLen  = 0;
    unsigned int converterInSize  = 0;
    unsigned int converterWinSize = 500;

    while (ret < frameLen) {
        if (!mSRCOutBuff) {
            if (converterWinSize < mWaveFileHolder->mFrameLen - mWaveFileHolder->mCurrentFrame) {
                converterInSize = converterWinSize;
            }
            else {
                converterInSize = mWaveFileHolder->mFrameLen - mWaveFileHolder->mCurrentFrame;
            }

            mSamplingRateConverter.apply(mWaveFileHolder->getFramePointer(mWaveFileHolder->mCurrentFrame)
                , converterInSize
                , &converterOut
                , &converterOutLen);

            mWaveFileHolder->mCurrentFrame += converterInSize;

            mSRCOutBuff = converterOut;
            mSRCOutFrameCurrent = 0;
            mSRCOutFrameLen = converterOutLen;
        }

        for (ret; ret < frameLen && mSRCOutFrameCurrent < mSRCOutFrameLen; ++ret) {
            for (int ch = 0; ch < mSystemChannels; ++ch) {
                *renderBuff++ = mSRCOutBuff[(mSRCOutFrameCurrent * mSystemChannels) + ch];
            }
            mSRCOutFrameCurrent++;
        }

        if (mSRCOutFrameLen <= mSRCOutFrameCurrent) {
            mSamplingRateConverter.releaseBuffer();
            mSRCOutBuff = nullptr;
            mSRCOutFrameLen = 0;
        }

        if (mWaveFileHolder->mFrameLen <= mWaveFileHolder->mCurrentFrame) {
            mWaveFileHolder->mCurrentFrame = 0;
            mPlay = false;
            break;
        }
    }

    return ret;
}

unsigned int SoundCoordinator::requestDataInternal(float** buff, unsigned int frameNum)
{
    unsigned int outFrameNum = 0;
    float*       outBuff     = *buff;
    unsigned int count       = 0;

    while (outFrameNum < frameNum) {
        SoundBuffer& writeSB = mSystemBuffer.mBuffer[mSystemBuffer.mWritePointer];

        if (mPlay) {
            float* writePoint = &writeSB.mBuffer[writeSB.mWritePointer];
            unsigned int ret = requestData(&writePoint, (writeSB.mBufferSize - writeSB.mWritePointer)/mSystemChannels);
            writeSB.mWritePointer += ret * mSystemChannels;
        }

        count = 0;
        for (unsigned int writePoint = writeSB.mWritePointer; writePoint < writeSB.mBufferSize; writePoint += mSystemChannels) {
            for (unsigned int ch = 0; ch < mSystemChannels; ++ch) {
                writeSB.mBuffer[writePoint + ch] = 0.0f;
                ++count;
            }
        }
        writeSB.mWritePointer += count;

        if (writeSB.mBufferSize <= writeSB.mWritePointer) {
            mSystemBuffer.mWritePointer = (mSystemBuffer.mWritePointer + 1) & 0x1;
        }

        SoundBuffer& readSB = mSystemBuffer.mBuffer[mSystemBuffer.mReadPointer];

        count = 0;
        for (unsigned int readPoint = readSB.mReadPointer
           ; readPoint < readSB.mWritePointer && outFrameNum < frameNum 
           ; readPoint += mSystemChannels) {

            for (unsigned int ch = 0; ch < mSystemChannels; ++ch) {
                *outBuff++ = readSB.mBuffer[readPoint + ch];
                ++count;
            }

            ++outFrameNum;
        }
        readSB.mReadPointer += count;

        if (readSB.mBufferSize <= readSB.mReadPointer) {
            readSB.mReadPointer  = 0;
            readSB.mWritePointer = 0;
            mSystemBuffer.mReadPointer = (mSystemBuffer.mReadPointer + 1) & 0x1;
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