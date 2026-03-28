#pragma once

#include <string>
#include "RoseAuraReturnCode.h"

using namespace RoseAuraReturnCode;

class ICentralLooper {
public:
	//////////////////////////////////////////////////////////
	// Internal Classes
	//////////////////////////////////////////////////////////
	class ITask {
	public:
		virtual void doTask() = 0;
		virtual void finish() = 0;
		virtual std::string getTaskName() = 0;

		virtual ~ITask() = default;
	protected:
		ITask() = default;
	};

	class IFrameSyncCallback {
	public:
		virtual void sync() = 0;

		virtual ~IFrameSyncCallback() = default;
	protected:
		IFrameSyncCallback() = default;
	};

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	virtual RARetCode start(unsigned int timeOfFrame) = 0;
	virtual RARetCode stop() = 0;
	virtual RARetCode enqueueTask(ITask* task) = 0;
	virtual RARetCode registerFrameSyncCallback(IFrameSyncCallback* cb) = 0;
	virtual RARetCode unregisterFrameSyncCallback(IFrameSyncCallback* cb) = 0;
};