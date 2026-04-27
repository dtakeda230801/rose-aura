#pragma once

#include <string>
#include "RoseAuraReturnCode.h"

class ICentralLooper {
public:
	//////////////////////////////////////////////////////////
	// Internal Classes
	//////////////////////////////////////////////////////////
	class ITask {
	public:
		virtual void		doTask()       = 0;
		virtual void		onTaskFinish() = 0;
		virtual std::string getTaskName()  = 0;

		virtual ~ITask() = default;
	protected:
		ITask() = default;
	};

	class IFrameSyncCallback {
	public:
		virtual void onFrameSync() = 0;

		virtual ~IFrameSyncCallback() = default;
	protected:
		IFrameSyncCallback() = default;
	};

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	virtual RARetCode start(uint32_t timeOfFrame)                         = 0;
	virtual RARetCode stop()                                              = 0;
	virtual RARetCode enqueueTask(ITask* task)                            = 0;
	virtual RARetCode registerFrameSyncCallback(IFrameSyncCallback* cb)   = 0;
	virtual RARetCode unregisterFrameSyncCallback(IFrameSyncCallback* cb) = 0;
};