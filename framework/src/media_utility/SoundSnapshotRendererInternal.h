#pragma once

#include "RoseAura.h"
#include "WaveFileHolderInternal.h"


class SoundSnapshotRendererInternal : public ISoundCoordinator::ISoundRenderer{
public:
	SoundSnapshotRendererInternal(RoseAura& ra, const char* waveFile);

	bool playSound();

	RARetCode requestData(unsigned int requestFrameLen, unsigned int* returnFrameLen, ISoundCoordinator::IDataWriter& writer);

	void onAudioStreamFinish();

	virtual ~SoundSnapshotRendererInternal() = default;

private:
	ISoundCoordinator&						mSoundCoordinator;
	std::unique_ptr<WaveFileHolderInternal> mWaveFileHolder;
};

