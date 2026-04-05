#include "SoundCoordinator.h"
#include "Utility.h"

#include <mmdeviceapi.h>
#include <combaseapi.h>
#include <audioclient.h>
#include <fstream>
#include <iostream>

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


void SoundCoordinator::renderToDevice()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    if (hr != S_OK) {
        Utility::printLog("CoInitializeEx fails");
        return;
    }

    while (mStarted) {
        IMMDeviceEnumerator* enumerator = nullptr;
        IMMDevice* device = nullptr;
        IAudioClient* audioClient = nullptr;
        IAudioRenderClient* renderClient = nullptr;

        hr = CoCreateInstance(
            __uuidof(MMDeviceEnumerator),
            nullptr,
            CLSCTX_ALL,
            IID_PPV_ARGS(&enumerator));
        
        if (hr != S_OK) {
            Utility::printLog("CoCreateInstance for IMMDeviceEnumerator fails");
            return;
        }

        hr = enumerator->GetDefaultAudioEndpoint(
            eRender,
            eConsole,
            &device);

        if (hr != S_OK) {
            Utility::printLog("GetDefaultAudioEndpoint fails");
            return;
        }

        hr = device->Activate(
            __uuidof(IAudioClient),
            CLSCTX_ALL,
            nullptr,
            (void**)&audioClient);

        if (hr != S_OK) {
            Utility::printLog("Activate fails");
            return;
        }

        WAVEFORMATEX* mixFormat = nullptr;
        hr = audioClient->GetMixFormat(&mixFormat);

        if (hr != S_OK) {
            Utility::printLog("GetMixFormat fails");
            return;
        }

        HANDLE audioEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (audioEvent == nullptr) {
            Utility::printLog("GetMixFormat fails");
            return;
        }

        //REFERENCE_TIME bufferDuration = 10000000; // 1 sec
        REFERENCE_TIME bufferDuration = 200000; // 50 msec

        hr = audioClient->Initialize(
            AUDCLNT_SHAREMODE_SHARED,
            AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
            bufferDuration,
            0,
            mixFormat,
            nullptr);

        if (hr != S_OK) {
            Utility::printLog("Initialize fails");
            return;
        }

        hr = audioClient->SetEventHandle(audioEvent);
        if (hr != S_OK) {
            Utility::printLog("SetEventHandle fails");
            return;
        }

        UINT32 bufferFrameCount;
        hr = audioClient->GetBufferSize(&bufferFrameCount);
        if (hr != S_OK) {
            Utility::printLog("GetBufferSize fails");
            return;
        }

        hr = audioClient->GetService(IID_PPV_ARGS(&renderClient));
        if (hr != S_OK) {
            Utility::printLog("GetService fails");
            return;
        }

        hr = audioClient->Start();
        if (hr != S_OK) {
            Utility::printLog("Start fails");
            return;
        }


        mSystemChannels     = mixFormat->nChannels;
        mSystemSamplingRate = mixFormat->nSamplesPerSec;


        while (mStarted)
        {
            WaitForSingleObject(audioEvent, INFINITE);

            UINT32 padding;
            hr = audioClient->GetCurrentPadding(&padding);
            if (hr != S_OK) {
                Utility::printLog("GetCurrentPadding fails");
                return;
            }

            UINT32 framesAvailable =
                bufferFrameCount - padding;

            if (framesAvailable == 0)
                continue;

            BYTE* data;
            hr = renderClient->GetBuffer(framesAvailable, &data);
            if (hr != S_OK) {
                Utility::printLog("GetBuffer fails");
                return;
            }
            float* out = (float*)data;

            for (UINT32 f = 0; f < framesAvailable ; ++f)
            {
                if (mPlay) {
                    unsigned int ret = requestData(&out, framesAvailable - f);
                    f += ret;
                }
                else {
                    for (int ch = 0; ch < mSystemChannels;ch++) {
                        *out++ = 0.0f;
                    }
                }
            }

            hr = renderClient->ReleaseBuffer(framesAvailable, 0);
            if (hr != S_OK) {
                Utility::printLog("ReleaseBuffer fails");
                return;
            }
        }

        Sleep(500);

        hr = audioClient->Stop();
        if (hr != S_OK) {
            Utility::printLog("Stop fails");
            return;
        }

        CoTaskMemFree(mixFormat);
        hr = renderClient->Release();
        if (hr != S_OK) {
            Utility::printLog("Release fails");
            return;
        }
        hr = audioClient->Release();
        if (hr != S_OK) {
            Utility::printLog("Release fails");
            return;
        }
        hr = device->Release();
        if (hr != S_OK) {
            Utility::printLog("Release fails");
            return;
        }
        hr = enumerator->Release();
        if (hr != S_OK) {
            Utility::printLog("Release fails");
            return;
        }

        CoUninitialize();
    }
}


SoundCoordinator::SoundCoordinator() :
      mStarted(false)
    , mSystemChannels(2)
    , mSystemSamplingRate(48000)
    , mPlay(false)
    , mSRCOutBuff(nullptr)
    , mSRCOutFrameCurrent(0)
    , mSRCOutFrameLen(0)
    , mConvertedCurrent(0)
    , mWaveFileHolder(nullptr)
{
    mWaveFileHolder = new Utility::WaveFileHolder("M:\\e\\works\\Dev\\test.wav");

    mSamplingRateConverter.reset();
    mSamplingRateConverter.setConfig(mWaveFileHolder->mSamplingRate, mWaveFileHolder->mChannels, 48000);

    float*       out;
    unsigned int outFrameLen;

    mSamplingRateConverter.apply(mWaveFileHolder->getFramePointer(0)
                               , mWaveFileHolder->mFrameLen
                               , &out
                               , &outFrameLen);
    mConvertedSamples.resize(outFrameLen * mWaveFileHolder->mChannels);

    for (int i = 0; i < outFrameLen * mWaveFileHolder->mChannels; i++) {
        mConvertedSamples[i] = *out++;
    }
    mSamplingRateConverter.releaseBuffer();
};

SoundCoordinator::~SoundCoordinator()
{
    if (mWaveFileHolder) {
        delete mWaveFileHolder;
    }
}

void SoundCoordinator::test()
{
    if (!mPlay) {
        mPlay = true;
        mConvertedCurrent = 0;
        mConvertedSamples.clear();

        mSRCOutBuff         = nullptr;
        mSRCOutFrameCurrent = 0;
        mSRCOutFrameLen     = 0;

        mSamplingRateConverter.reset();
        mSamplingRateConverter.setConfig(mWaveFileHolder->mSamplingRate
                                       , mSystemChannels
                                       , mSystemSamplingRate);
    }
}

/*
unsigned int MediaCoordinator::requestData(float** buff, unsigned int size, unsigned int chs)
{
    float* b = *buff;
    unsigned int ret = 0;
    unsigned int inFrameLen = mConvertedSamples.size() / chs;


    for (ret; ret < size && mConvertedCurrent < inFrameLen; ++ret) {
        for (int ch = 0; ch < chs; ++ch) {
            *b++ = mConvertedSamples[(mConvertedCurrent * chs) + ch];
        }
        mConvertedCurrent++;
    }

    if (inFrameLen <= mConvertedCurrent) {
        mConvertedCurrent = 0;
        mPlay = false;
    }

    return ret;
}
*/

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

    Utility::printLog("request:(%d) write:(%d)", frameLen, ret);

    return ret;
}


/*
unsigned int MediaCoordinator::requestData(float** buff, unsigned int size, unsigned int chs)
{
    float*          b  = *buff;
    unsigned int    ret = 0;

    for (ret; ret < size && mWavData.current < mWavData.samples.size(); ++ret) {
        for (int ch = 0; ch < chs; ++ch) {
            *b++ = mWavData.samples[mWavData.current++];
        }
    }

    if (mWavData.samples.size() <= mWavData.current) {
        mWavData.current = 0;
        mPlay = false;
    }

    Utility::printLog("Write:%d",mWavData.current);

    return ret;
}
*/
