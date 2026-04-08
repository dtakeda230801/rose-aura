#pragma once

#include "RoseAuraReturnCode.h"

using namespace RoseAuraReturnCode;

class ISoundCoordinator {
public:
	//////////////////////////////////////////////////////////
	// Internal Classes
	//////////////////////////////////////////////////////////
	class IDataWriter {
	public:
		virtual RARetCode write(float* writeData, unsigned int requestFrameLen) = 0;
	};

	class ISoundRenderer {
	public:
		virtual RARetCode requestData(unsigned int requestFrameLen, unsigned int* returnFrameLen, IDataWriter& writer) = 0;

		virtual ~ISoundRenderer() = default;
	protected:
		ISoundRenderer() = default;
	};

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	virtual RARetCode start() = 0;
	virtual RARetCode stop()  = 0;

	virtual unsigned int getSystemSamplingRate() = 0;
	virtual unsigned int getSystemChannels()     = 0;

	virtual RARetCode registerRenderer(ISoundRenderer* renderer) = 0;
};