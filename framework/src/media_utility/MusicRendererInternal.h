#pragma once

#include "RoseAura.h"

#include "OpusFileHolderInternal.h"
#include "MediaUtility.h"

using namespace RoseAuraMediaUtility;

class MusicRendererInternal :
	  public ISoundCoordinator::ISoundRenderer
	, public PreRenderThread
{
public:

	bool playMusic();
	bool stopPlaying();
	void setJumpPoint(uint64_t point, uint64_t to);

	MusicRendererInternal(RoseAura& ra, const char* musicFile);
	virtual ~MusicRendererInternal();
private:
	void	  doWork();
	RARetCode requestData(unsigned int requestFrameLen, unsigned int* returnFrameLen, ISoundCoordinator::IDataWriter& writer);
	void	  onAudioStreamFinish();
	void      startDecodeThread();
	void	  termDecodeThread();

	std::unique_ptr<OpusFileHolderInternal> mOpusFileHolder;

	RoseAura&	mRa;
	bool		mPlay;
	bool		mNoFinish;

};