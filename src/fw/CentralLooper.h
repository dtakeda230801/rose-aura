#pragma once

#include <queue>
#include <mutex>
#include <string>

class CentralLooper {
public:
	class ITask {
	public:
		virtual int			doTask()    = 0;
		virtual int			finish()    = 0;
		virtual std::string getTaskId() = 0;

		virtual ~ITask() = default;
	protected:
		ITask() = default;

	};

	class IFrameSyncCallback {
	public:
		virtual int sync()= 0 ;

		virtual ~IFrameSyncCallback() = default;
	protected:
		IFrameSyncCallback() = default;
	};

	static CentralLooper* getInstance();
	static void destroyInstance();

	int start(int timeOfFrame);
	int stop();
	int setTask(ITask* task);
	int registerFrameSyncCallback(IFrameSyncCallback* cb);

private:
	CentralLooper();
	virtual ~CentralLooper() = default;

	static CentralLooper* sInstance;

	void  run();
	ITask* dequeue();

	std::queue<ITask*> mTaskQueue;
	std::mutex		   mMutex;
	std::thread		   mThread;

	std::atomic<bool>  mStarted;

	int				   mTimeOfFrame;
	
	std::vector<IFrameSyncCallback*>
				       mFrameSyncCallbacks;

};



