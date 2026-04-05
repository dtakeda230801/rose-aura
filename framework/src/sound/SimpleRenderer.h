#pragma once

#include "ISoundCoordinator.h"
#include "SoundCoordinator.h"

class SimpleRenderer : public SoundCoordinator::IRenderer
{
public:
	unsigned int requestData(float** buff, unsigned int frameLen);

    SimpleRenderer(ISoundCoordinator::SoundDescriptor descriptor);
    virtual ~SimpleRenderer() = default;
};
