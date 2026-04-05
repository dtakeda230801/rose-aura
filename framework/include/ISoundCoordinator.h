#pragma once

#include "RoseAuraReturnCode.h"

using namespace RoseAuraReturnCode;

class ISoundCoordinator {
public:
	//////////////////////////////////////////////////////////
	// Internal Classes
	//////////////////////////////////////////////////////////
	using RequestDataHandler = unsigned int(*)(float**, unsigned int);

	struct SoundDescriptor {
		RequestDataHandler mHandler;
		unsigned int	   mSamplingRate;
		unsigned int	   mChannels;
	};

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	virtual RARetCode start() = 0;
	virtual RARetCode stop()  = 0;

	virtual unsigned int getSystemSamplingRate() = 0;
	virtual unsigned int getSystemChannels()     = 0;

	virtual RARetCode playOneShut(SoundDescriptor descriptor) = 0;

	virtual void test() = 0;
};