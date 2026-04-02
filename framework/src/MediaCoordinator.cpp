#include "MediaCoordinator.h"
#include "Utility.h"

#include <mmdeviceapi.h>
#include <combaseapi.h>
#include <audioclient.h>
#include <fstream>
#include <iostream>

#pragma comment(lib, "ole32.lib")


RARetCode MediaCoordinator::start()
{
    if (mStarted) {
        return RARetCode::RET_ERR_INVALID_STATE;
    }

    mStarted = true;

    mThread = std::thread(&MediaCoordinator::renderToDevice, this);
    return RARetCode::RET_OK;
}

RARetCode MediaCoordinator::stop()
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


void MediaCoordinator::renderToDevice()
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
        REFERENCE_TIME bufferDuration = 200000; // 50 sec

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

        int channels = mixFormat->nChannels;

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
                    unsigned int ret = requestData(&out, framesAvailable - f, channels);
                    f += ret;
                }
                else {
                    for (int ch = 0; ch < channels;ch++) {
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


MediaCoordinator::MediaCoordinator() :
      mStarted(false)
    , mPlay(false)
{
    if (!loadWav("M:\\e\\works\\Dev\\test.wav", mWavData)) {
        std::cout << "failed load wav\n";
        return;
    }
};

void MediaCoordinator::test()
{
    if (!mPlay) {
        mPlay = true;
    }
}

bool MediaCoordinator::loadWav(const char* path, WavData& out)
{
    std::ifstream waveFile(path, std::ios::binary);
    if (!waveFile) return false;

    char riff[4];
    waveFile.read(riff, 4);
    char size[4];
    waveFile.read(size, 4);
    char wave[4];
    waveFile.read(wave, 4);

    char chunk[4];
    int fmtSize;

    // find fmt
    while (true) {
        waveFile.read(chunk, 4);
        waveFile.read((char*)&fmtSize, 4);
        if (memcmp(chunk, "fmt ", 4) == 0)
            break;
        waveFile.ignore(fmtSize);
    }

    short audioFormat;
    short channels;
    int sampleRate;
    waveFile.read((char*)&audioFormat, 2);
    waveFile.read((char*)&channels, 2);
    waveFile.read((char*)&sampleRate, 4);
    waveFile.ignore(6);
    short bitsPerSample;
    waveFile.read((char*)&bitsPerSample, 2);

    if (fmtSize > 16)
        waveFile.ignore(fmtSize - 16);

    // find data
    int dataSize;
    while (true) {
        waveFile.read(chunk, 4);
        waveFile.read((char*)&dataSize, 4);
        if (memcmp(chunk, "data", 4) == 0)
            break;
        waveFile.ignore(dataSize);
    }

    int sampleCount = dataSize / (bitsPerSample / 8);

    out.samples.resize(sampleCount);

    if (bitsPerSample == 16) {
        std::vector<short> tmp(sampleCount);
        waveFile.read((char*)tmp.data(), dataSize);

        for (int i = 0; i < sampleCount; ++i)
            out.samples[i] = tmp[i] / 32768.0f;
    }
    else if (bitsPerSample == 32) {
        waveFile.read((char*)out.samples.data(), dataSize);
    }
    else {
        return false;
    }

    out.channels   = channels;
    out.sampleRate = sampleRate;
    out.current    = 0;
    return true;

}

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
