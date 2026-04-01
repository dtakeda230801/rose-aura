#include "MediaCoordinator.h"
#include "Utility.h"

#include <mmdeviceapi.h>
#include <combaseapi.h>
#include <audioclient.h>
#include <fstream>
#include <iostream>

#pragma comment(lib, "ole32.lib")

void MediaCoordinator::renderAudioThread()
{
    HRESULT ret = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    if (ret != S_OK) {
        Utility::printLog("CoInitializeEx fails");
        return;
    }

    while (mStarted) {
        IMMDeviceEnumerator* enumerator = nullptr;
        IMMDevice* device = nullptr;
        IAudioClient* audioClient = nullptr;
        IAudioRenderClient* renderClient = nullptr;

        CoCreateInstance(
            __uuidof(MMDeviceEnumerator),
            nullptr,
            CLSCTX_ALL,
            IID_PPV_ARGS(&enumerator));
        
        if (!enumerator) {
            Utility::printLog("CoCreateInstance for IMMDeviceEnumerator fails");
            return;
        }

        enumerator->GetDefaultAudioEndpoint(
            eRender,
            eConsole,
            &device);

        if (!device) {
            Utility::printLog("GetDefaultAudioEndpoint fails");
            return;
        }

        device->Activate(
            __uuidof(IAudioClient),
            CLSCTX_ALL,
            nullptr,
            (void**)&audioClient);

        if (!audioClient) {
            Utility::printLog("Activate fails");
            return;
        }

    }



}



void MediaCoordinator::test()
{
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    WavData wav;
    if (!loadWav("M:\\e\\works\\Dev\\test.wav", wav)) {
        std::cout << "failed load wav\n";
        return;
    }

    IMMDeviceEnumerator* enumerator   = nullptr;
    IMMDevice*           device       = nullptr;
    IAudioClient*        audioClient  = nullptr;
    IAudioRenderClient*  renderClient = nullptr;

    CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        IID_PPV_ARGS(&enumerator));

    enumerator->GetDefaultAudioEndpoint(
        eRender,
        eConsole,
        &device);

    device->Activate(
        __uuidof(IAudioClient),
        CLSCTX_ALL,
        nullptr,
        (void**)&audioClient);

    WAVEFORMATEX* mixFormat = nullptr;
    audioClient->GetMixFormat(&mixFormat);

    HANDLE audioEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

    REFERENCE_TIME bufferDuration = 10000000; // 1 sec

    audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
        bufferDuration,
        0,
        mixFormat,
        nullptr);

    audioClient->SetEventHandle(audioEvent);

    UINT32 bufferFrameCount;
    audioClient->GetBufferSize(&bufferFrameCount);

    audioClient->GetService(
        IID_PPV_ARGS(&renderClient));

    audioClient->Start();

    size_t cursor = 0;
    int channels = mixFormat->nChannels;

    while (cursor < wav.samples.size())
    {
        WaitForSingleObject(audioEvent, INFINITE);

        UINT32 padding;
        audioClient->GetCurrentPadding(&padding);

        UINT32 framesAvailable =
            bufferFrameCount - padding;

        BYTE* data;
        renderClient->GetBuffer(framesAvailable, &data);

        float* out = (float*)data;

        for (UINT32 f = 0; f < framesAvailable; ++f)
        {
            for (int ch = 0; ch < channels; ++ch)
            {
                if (cursor < wav.samples.size())
                    *out++ = wav.samples[cursor++];
                else
                    *out++ = 0.0f;
            }
        }

        renderClient->ReleaseBuffer(framesAvailable, 0);
    }

    Sleep(500);

    audioClient->Stop();

    CoTaskMemFree(mixFormat);
    renderClient->Release();
    audioClient->Release();
    device->Release();
    enumerator->Release();

    CoUninitialize();

}


bool MediaCoordinator::loadWav(const char* path, WavData& out)
{
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;

    char riff[4];
    f.read(riff, 4);
    f.ignore(4); // size
    f.ignore(4); // WAVE

    char chunk[4];
    int fmtSize;

    // find fmt
    while (true) {
        f.read(chunk, 4);
        f.read((char*)&fmtSize, 4);
        if (memcmp(chunk, "fmt ", 4) == 0)
            break;
        f.ignore(fmtSize);
    }

    short audioFormat;
    short channels;
    int sampleRate;
    f.read((char*)&audioFormat, 2);
    f.read((char*)&channels, 2);
    f.read((char*)&sampleRate, 4);
    f.ignore(6);
    short bitsPerSample;
    f.read((char*)&bitsPerSample, 2);

    if (fmtSize > 16)
        f.ignore(fmtSize - 16);

    // find data
    int dataSize;
    while (true) {
        f.read(chunk, 4);
        f.read((char*)&dataSize, 4);
        if (memcmp(chunk, "data", 4) == 0)
            break;
        f.ignore(dataSize);
    }

    int sampleCount = dataSize / (bitsPerSample / 8);

    out.samples.resize(sampleCount);

    if (bitsPerSample == 16) {
        std::vector<short> tmp(sampleCount);
        f.read((char*)tmp.data(), dataSize);

        for (int i = 0; i < sampleCount; ++i)
            out.samples[i] = tmp[i] / 32768.0f;
    }
    else if (bitsPerSample == 32) {
        f.read((char*)out.samples.data(), dataSize);
    }
    else {
        return false;
    }

    out.channels = channels;
    out.sampleRate = sampleRate;
    return true;

}