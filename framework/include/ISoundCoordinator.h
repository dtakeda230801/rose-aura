#pragma once

#include <cstdint>

#include "RoseAuraReturnCode.h"

using namespace RoseAuraReturnCode;

class ISoundCoordinator {
public:
	//////////////////////////////////////////////////////////
	// Internal Classes
	//////////////////////////////////////////////////////////
	class IDataWriter {
	public:
		virtual RARetCode write(float* writeData, uint32_t requestFrameLen) = 0;
	};

	class ISoundRenderer {
	public:
		virtual RARetCode requestData(uint32_t requestFrameLen, uint32_t* returnFrameLen, IDataWriter& writer) = 0;
		virtual ~ISoundRenderer() = default;
	protected:
		ISoundRenderer() = default;
	};

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	virtual RARetCode start() = 0;
	virtual RARetCode stop()  = 0;

	virtual uint32_t getSystemSamplingRate() = 0;
	virtual uint32_t getSystemChannels()     = 0;
	virtual uint32_t getDelayTime()          = 0;

	virtual RARetCode registerRenderer(ISoundRenderer* renderer)   = 0;
	virtual RARetCode unregisterRenderer(ISoundRenderer* renderer) = 0;
};