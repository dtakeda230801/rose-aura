#include "sound/SimpleRenderer.h"

unsigned int SimpleRenderer::requestData(float** buff, unsigned int frameLen)
{
	return mDesctiptor.mHandler(buff, frameLen);
}

SimpleRenderer::SimpleRenderer(ISoundCoordinator::SoundDescriptor descriptor) :
	IRenderer(descriptor)
{
}

