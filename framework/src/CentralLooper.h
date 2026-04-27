#pragma once

#include <queue>
#include <mutex>
#include <memory>

#include "ICentralLooper.h"
#include "RoseAuraReturnCode.h"

class CentralLooper : public ICentralLooper {
public:
	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	RARetCode start(uint32_t  timeOfFrame);
	RARetCode stop();
	RARetCode enqueueTask(ITask* task);
	RARetCode registerFrameSyncCallback(IFrameSyncCallback* cb);
	RARetCode unregisterFrameSyncCallback(IFrameSyncCallback* cb);

	CentralLooper();
	virtual ~CentralLooper() = default;

private:
	void  run();
	ITask* dequeue();

	std::queue<ITask*> mTaskQueue;
	std::mutex		   mMutex;
	std::thread		   mThread;

	std::atomic<bool>  mStarted;

	uint32_t		   mTimeOfFrame;
	
	std::vector<IFrameSyncCallback*>
				       mFrameSyncCallbacks;

};



