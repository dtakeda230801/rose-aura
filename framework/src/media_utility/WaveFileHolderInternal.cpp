#include <fstream>
#include <vector>

#include "WaveFileHolderInternal.h"

using namespace RoseAuraMediaUtility;

WaveFileHolderInternal::WaveFileHolderInternal(const char* path) :
    mChannels(0)
    , mSamplingRate(0)
    , mFrameLen(0)
    , mCurrentFrame(0)
    , mData(nullptr)

{
    char  riff[4];
    char  size[4];
    char  wave[4];
    char  chunk[4];
    int   chunkSize;
    short audioFormat;
    short channels;
    int   sampleRate;
    short bitsPerSample;
    int   dataSize;
    int   sampleCount;

    std::ifstream waveFile(path, std::ios::binary);
    if (!waveFile)
    {
        return;
    }

    waveFile.read(riff, 4);
    waveFile.read(size, 4);
    waveFile.read(wave, 4);

    while (true) {
        waveFile.read(chunk, 4);
        waveFile.read((char*)&chunkSize, 4);
        if (memcmp(chunk, "fmt ", 4) == 0) {
            break;
        }
        waveFile.ignore(chunkSize);
    }

    waveFile.read((char*)&audioFormat, 2);
    waveFile.read((char*)&channels, 2);
    waveFile.read((char*)&sampleRate, 4);
    waveFile.ignore(6);
    waveFile.read((char*)&bitsPerSample, 2);

    if (chunkSize > 16) {
        waveFile.ignore(chunkSize - 16);
    }

    while (true) {
        waveFile.read(chunk, 4);
        waveFile.read((char*)&chunkSize, 4);
        if (memcmp(chunk, "data", 4) == 0) {
            break;
        }
        waveFile.ignore(chunkSize);
    }

    dataSize = chunkSize;

    sampleCount = dataSize / (bitsPerSample / 8);

    mData = new float[sampleCount];

    if (bitsPerSample == 16) {
        std::vector<short> tmp(sampleCount);
        waveFile.read((char*)tmp.data(), dataSize);

        for (int i = 0; i < sampleCount; ++i)
            mData[i] = tmp[i] / 32768.0f;
    }
    else if (bitsPerSample == 32) {
        waveFile.read((char*)mData, dataSize);
    }
    else {
        return;
    }

    mChannels = channels;
    mSamplingRate = sampleRate;
    mFrameLen = static_cast<uint32_t>(sampleCount) / channels;
    mCurrentFrame = 0;
}

float* WaveFileHolderInternal::getCurrentFramePointer()
{
    return &mData[mCurrentFrame * mChannels];
}

void WaveFileHolderInternal::moveCurrentFramePointer(uint32_t frameLen)
{
    mCurrentFrame += frameLen;

    if (frameLen > mFrameLen - 1) {
        mCurrentFrame = mFrameLen - 1;
    }
}

uint32_t WaveFileHolderInternal::getFrameLen()
{
    return mFrameLen;
}

uint32_t WaveFileHolderInternal::getRemainFrameLen()
{
    return mFrameLen - mCurrentFrame;
}


uint32_t WaveFileHolderInternal::getSamplingRate()
{
    return mSamplingRate;
}

uint32_t WaveFileHolderInternal::getChannelNum()
{
    return mChannels;
}

void WaveFileHolderInternal::reset()
{
    mCurrentFrame = 0;
}


WaveFileHolderInternal::~WaveFileHolderInternal()
{
    if (mData) {
        delete mData;
    }
}
