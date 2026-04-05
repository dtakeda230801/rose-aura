#pragma once

#include "ISoundCoordinator.h"
#include "SoundCoordinator.h"

class SRCRenderer : public SoundCoordinator::IRenderer
{
public:
    unsigned int requestData(float** buff, unsigned int frameLen);

    SRCRenderer(ISoundCoordinator::SoundDescriptor descriptor);
    virtual ~SRCRenderer() = default;

private:
    std::unique_ptr<SamplingRateConverter> mSamplingRateConverter;

    float*          mSRCOutBuff;
    unsigned int    mSRCOutFrameCurrent;
    unsigned int    mSRCOutFrameLen;
};
