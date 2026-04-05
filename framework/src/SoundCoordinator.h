#pragma once

#include <vector>
#include <thread>

#include "ISoundCoordinator.h"
#include "sound/SamplingRateConverter.h"
#include "Utility.h"

class SoundCoordinator : public ISoundCoordinator {
public:
	//////////////////////////////////////////////////////////
	// Internal Classes
	//////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	RARetCode start();
	RARetCode stop();

	void test();

	SoundCoordinator();
	virtual ~SoundCoordinator();

private:
	unsigned int requestData(float** buff,unsigned int frameNum);

	void renderToDevice();

	std::thread		mThread;
	bool			mStarted;

	unsigned int	mSystemChannels;
	unsigned int	mSystemSamplingRate;

	bool			mPlay;
	Utility::WaveFileHolder*
					mWaveFileHolder;

	SamplingRateConverter
					mSamplingRateConverter;

	float*          mSRCOutBuff;
	unsigned int    mSRCOutFrameCurrent;
	unsigned int    mSRCOutFrameLen;

	std::vector<float>
		            mConvertedSamples;
	unsigned int    mConvertedCurrent;
};