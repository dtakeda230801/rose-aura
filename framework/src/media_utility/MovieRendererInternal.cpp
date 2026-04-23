#include "MovieRendererInternal.h"


MovieRendererInternal::MovieRendererInternal(RoseAura& ra, const int8_t* movieFile, MovieRenderer::IMovieRendererCallback* cb) :
      mRoseAura(ra)
    , mCallback(cb)
{
}

bool MovieRendererInternal::start()
{
    return false;

}

bool MovieRendererInternal::stop()
{
    return false;
}

MovieRendererInternal::~MovieRendererInternal()
{

}

void MovieRendererInternal::preprocess()
{

}

void MovieRendererInternal::render()
{

}

RARetCode MovieRendererInternal::requestData(uint32_t requestFrameLen, uint32_t* returnFrameLen, ISoundCoordinator::IDataWriter& writer)
{
    return RARetCode::RET_OK;
}

void MovieRendererInternal::onAudioStreamFinish()
{

}

void MovieRendererInternal::doWork()
{

}
