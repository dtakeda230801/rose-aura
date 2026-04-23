#pragma once

#include "RoseAura.h"
#include "MediaUtility.h"

using namespace RoseAuraMediaUtility;

class MovieRendererInternal :
	  public IGraphicsManager::IGraphicsRenderer
	, public ISoundCoordinator::ISoundRenderer
	, public PreRenderThread
{
public:
	/////////////////////////////
	MovieRendererInternal(RoseAura& ra, const int8_t* movieFile, MovieRenderer::IMovieRendererCallback* cb);

	bool start();
	bool stop();

	~MovieRendererInternal();


	/////////////////////////////
	void preprocess();
	void render();

	RARetCode requestData(uint32_t requestFrameLen, uint32_t* returnFrameLen, ISoundCoordinator::IDataWriter& writer);
	void      onAudioStreamFinish();

	void doWork();

private:
	RoseAura& mRoseAura;

	MovieRenderer::IMovieRendererCallback*
		mCallback;
};
