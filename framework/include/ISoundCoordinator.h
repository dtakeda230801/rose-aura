#pragma once

#include "RoseAuraReturnCode.h"

using namespace RoseAuraReturnCode;

class ISoundCoordinator {
public:
	//////////////////////////////////////////////////////////
	// Internal Classes
	//////////////////////////////////////////////////////////
	using RequestDataFunction = unsigned int(*)(float**, unsigned int);

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	virtual RARetCode start() = 0;
	virtual RARetCode stop()  = 0;

	virtual unsigned int getSystemSamplingRate() = 0;
	virtual unsigned int getChannels()           = 0;

	virtual RARetCode playOneShut(RequestDataFunction func) = 0;

	virtual void test() = 0;
};