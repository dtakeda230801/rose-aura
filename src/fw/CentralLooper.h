#pragma once

#include <queue>
#include <mutex>
#include <string>
#include <memory>

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

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	static void				createInstance();
	static void				destroyInstance();
	static CentralLooper&	getInstance();

	int start(int timeOfFrame);
	int stop();
	int setTask(ITask* task);
	int registerFrameSyncCallback(IFrameSyncCallback* cb);

	virtual ~CentralLooper() = default;

private:
	CentralLooper();

	static std::unique_ptr<CentralLooper> mInstance;

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



