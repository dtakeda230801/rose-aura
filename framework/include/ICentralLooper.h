#pragma once

#include <string>

class ICentralLooper {
public:
	//////////////////////////////////////////////////////////
	// Internal Classes
	//////////////////////////////////////////////////////////
	class ITask {
	public:
		virtual int			doTask() = 0;
		virtual int			finish() = 0;
		virtual std::string getTaskId() = 0;

		virtual ~ITask() = default;
	protected:
		ITask() = default;
	};

	class IFrameSyncCallback {
	public:
		virtual int sync() = 0;

		virtual ~IFrameSyncCallback() = default;
	protected:
		IFrameSyncCallback() = default;
	};

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	virtual int start(int timeOfFrame) = 0;
	virtual int stop() = 0;
	virtual int setTask(ITask* task) = 0;
	virtual int registerFrameSyncCallback(IFrameSyncCallback* cb) = 0;
};