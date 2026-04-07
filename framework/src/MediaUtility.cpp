#include <iostream>
#include <fstream>


#include "MediaUtility.h"

using namespace RoseAuraMediaUtility;

WaveFileHolder::WaveFileHolder(const char* path) :
      mChannels(0)
    , mSamplingRate(0)
    , mFrameLen(0)
    , mCurrentFrame(0)
    , mData(nullptr)

{
    std::ifstream waveFile(path, std::ios::binary);
    if (!waveFile) return;

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
    int   sampleRate;
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

    mData = new float[sampleCount];

    if (bitsPerSample == 16) {
        std::vector<short> tmp(sampleCount);
        waveFile.read((char*)tmp.data(), dataSize);

        for (int i = 0; i < sampleCount; ++i)
            mSamples[i] = tmp[i] / 32768.0f;
    }
    else if (bitsPerSample == 32) {
        waveFile.read((char*)mSamples.data(), dataSize);
    }
    else {
        return;
    }

    mChannels = channels;
    mSamplingRate = sampleRate;
    mFrameLen = static_cast<unsigned int>(mSamples.size()) / channels;
    mCurrentFrame = 0;
}

float* WaveFileHolder::getFramePointer(unsigned int frame)
{
    return &mSamples[frame * mChannels];
}

float* WaveFileHolder::getCurrentFramePointer()
{

}

unsigned int WaveFileHolder::getFrameLen()
{

}

unsigned int WaveFileHolder::getSamplingRate()
{

}

unsigned int WaveFileHolder::getChannelNum()
{
}

void WaveFileHolder::reset()
{

}
