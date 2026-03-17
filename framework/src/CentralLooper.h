#pragma once

#include <queue>
#include <mutex>
#include <memory>

#include "ICentralLooper.h"

class CentralLooper : public ICentralLooper {
public:
	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	int start(int timeOfFrame);
	int stop();
	int setTask(ITask* task);
	int registerFrameSyncCallback(IFrameSyncCallback* cb);

	CentralLooper();
	virtual ~CentralLooper() = default;

private:
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



