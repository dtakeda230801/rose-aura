#include <iostream>
#include <fstream>


#include "Utility.h"
#include "MediaUtility.h"

using namespace RoseAuraMediaUtility;

////////////////////////////////////////////////////
////////////////////////////////////////////////////
WaveFileHolder::WaveFileHolder(const char* path) :
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

    mChannels     = channels;
    mSamplingRate = sampleRate;
    mFrameLen     = static_cast<unsigned int>(sampleCount) / channels;
    mCurrentFrame = 0;
}

float* WaveFileHolder::getCurrentFramePointer()
{
    return &mData[mCurrentFrame * mChannels];
}

void WaveFileHolder::moveCurrentFramePointer(unsigned int frameLen)
{
    mCurrentFrame += frameLen;

    if (frameLen > mFrameLen - 1) {
        mCurrentFrame = mFrameLen - 1;
    }
}

unsigned int WaveFileHolder::getFrameLen()
{
    return mFrameLen;
}

unsigned int WaveFileHolder::getRemainFrameLen()
{
    return mFrameLen - mCurrentFrame;
}


unsigned int WaveFileHolder::getSamplingRate()
{
    return mSamplingRate;
}

unsigned int WaveFileHolder::getChannelNum()
{
    return mChannels;
}

void WaveFileHolder::reset()
{
    mCurrentFrame = 0;
}


WaveFileHolder::~WaveFileHolder()
{
    if (mData) {
        delete mData;
    }
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
OpusFileHolder::OpusFileHolder(const char* path) :
      mFile(nullptr)
    , mChannels(0)
    , mBuffer(nullptr)
    , mFrameLen(0)
    , mReadPointer(0)

{
    int ret;
    mFile = op_open_file(path, &ret);
    if (!mFile) {
        Utility::printLog("op_open_file returns error(%d)", ret);
        return;
    }
    
    ret = op_channel_count(mFile, -1);
    if (ret == OP_EINVAL) {
        Utility::printLog("op_channel_count returns error(%d)", ret);
        return;
    }
    mChannels = static_cast<unsigned int>(ret);

    mBuffer = new float[BUFF_FRAME_LEN * mChannels];
}

bool OpusFileHolder::decode()
{
    bool ret = true;

    if (mFrameLen == 0) {
        int ret;
        ret = op_read_float(mFile, mBuffer, BUFF_FRAME_LEN * mChannels, nullptr);
        if (ret > 0) {
            mFrameLen = ret;
        }

        if (ret == 0) {
            ret = false;
        }
    }

    return ret;
}

void OpusFileHolder::getCurrentPointer(float*& data, unsigned int* frameLen)
{

    data      = mBuffer + mReadPointer * mChannels;
    *frameLen = mFrameLen;
}

void OpusFileHolder::moveReadPointer(unsigned int frameLen)
{
    mReadPointer += frameLen;
    if (mReadPointer >= mFrameLen) {
        mFrameLen    = 0;
        mReadPointer = 0;
    }
}

unsigned int OpusFileHolder::getChannels()
{
    return mChannels;
}

void OpusFileHolder::reset()
{
    op_pcm_seek(mFile, 0);
    mFrameLen    = 0;
    mReadPointer = 0;
}

OpusFileHolder::~OpusFileHolder()
{
    if (mFile) {
        op_free(mFile);
    }

    if (mBuffer) {
        delete[] mBuffer;
    }
}
