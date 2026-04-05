#include "SRCRenderer.h"

unsigned int SRCRenderer::requestData(float** buff, unsigned int frameLen)
{
    unsigned int ret  = 0;
    float* renderBuff = *buff;

    float*       sourceBuffer;
    unsigned int sourceOutLen;

    float*       converterOut       = nullptr;
    unsigned int converterOutLen    = 0;
    unsigned int converterWinSize   = 500;

    ISoundCoordinator::RequestDataHandler handler = mDesctiptor.mHandler;

    while (ret < frameLen) {
        if (!mSRCOutBuff) {
            sourceOutLen = handler(&sourceBuffer, converterWinSize);

            mSamplingRateConverter->apply(sourceBuffer
                , sourceOutLen
                , &converterOut
                , &converterOutLen);

            mSRCOutBuff         = converterOut;
            mSRCOutFrameCurrent = 0;
            mSRCOutFrameLen     = converterOutLen;
        }

        for (ret; ret < frameLen && mSRCOutFrameCurrent < mSRCOutFrameLen; ++ret) {
            for (int ch = 0; ch < mSystemChannels; ++ch) {
                *renderBuff++ = mSRCOutBuff[(mSRCOutFrameCurrent * mSystemChannels) + ch];
            }
            mSRCOutFrameCurrent++;
        }

        if (mSRCOutFrameLen <= mSRCOutFrameCurrent) {
            mSamplingRateConverter->releaseBuffer();
            mSRCOutBuff = nullptr;
            mSRCOutFrameLen = 0;
        }
    }
    return ret;
}

SRCRenderer::SRCRenderer(ISoundCoordinator::SoundDescriptor descriptor) :
	  IRenderer(descriptor)
    , mSRCOutBuff(nullptr)
    , mSRCOutFrameCurrent(0)
    , mSRCOutFrameLen(0)
{
    mSamplingRateConverter = std::make_unique<SamplingRateConverter>();
    mSamplingRateConverter->reset();
    mSamplingRateConverter->setConfig(mDesctiptor.mSamplingRate, mDesctiptor.mChannels, mSystemSamplingRate);
}

