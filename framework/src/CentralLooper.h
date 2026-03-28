#pragma once

#include <queue>
#include <mutex>
#include <memory>

#include "ICentralLooper.h"
#include "RoseAuraReturnCode.h"

using namespace RoseAuraReturnCode;

class CentralLooper : public ICentralLooper {
public:
	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	RARetCode start(unsigned int  timeOfFrame);
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

	int				   mTimeOfFrame;
	
	std::vector<IFrameSyncCallback*>
				       mFrameSyncCallbacks;

};



