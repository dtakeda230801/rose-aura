#include "SoundSnapshotRendererInternal.h"
#include "RoseAura.h"
#include "Utility.h"

SoundSnapshotRendererInternal::SoundSnapshotRendererInternal(RoseAura& ra, const char* waveFile) :
	  mSoundCoordinator(ra.getSoundCoordinator())
	, mWaveFileHolder(std::make_unique<WaveFileHolderInternal>(waveFile))
{
}

bool SoundSnapshotRendererInternal::playSound()
{
	RARetCode ret = mSoundCoordinator.registerRenderer(this);

	if (ret == RARetCode::RET_OK) {
		return true;
	}

	return false;
}

RARetCode SoundSnapshotRendererInternal::requestData(unsigned int requestFrameLen, unsigned int* returnFrameLen, ISoundCoordinator::IDataWriter& writer)
{

	if (requestFrameLen <= mWaveFileHolder->getRemainFrameLen()) {
		*returnFrameLen = requestFrameLen;
	}
	else {
		*returnFrameLen = mWaveFileHolder->getRemainFrameLen();
	}

	writer.write(mWaveFileHolder->getCurrentFramePointer(), *returnFrameLen);

	mWaveFileHolder->moveCurrentFramePointer(*returnFrameLen);

	if (mWaveFileHolder->getRemainFrameLen() == 0) {
		mWaveFileHolder->reset();
		return RARetCode::RET_END_OF_CONTENT;
	}

	return RARetCode::RET_OK;
}

void SoundSnapshotRendererInternal::onAudioStreamFinish()
{
	Utility::printLog("SoundSnapshotRendererInternal onFinish");
}

