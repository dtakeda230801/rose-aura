#include "CentralLooper.h"
#include "Utility.h"
#include <cstddef>
#include <memory>

////////////////////////////////////
// APIs
////////////////////////////////////
RARetCode CentralLooper::start(unsigned int  timeOfFrame)
{

	if (mStarted) {
		return RARetCode::RET_ERR_INVALID_STATE;
	}

	mTimeOfFrame = timeOfFrame;
	mStarted     = true;

	mThread = std::thread(&CentralLooper::run, this);
	return RARetCode::RET_OK;
}

RARetCode CentralLooper::stop()
{
	if (!mStarted) {
		return RARetCode::RET_ERR_INVALID_STATE;
	}

	{
		std::lock_guard<std::mutex> lock(mMutex);
		mStarted = false;
	}

	if (mThread.joinable()) {
		mThread.join();
	}
	return RARetCode::RET_OK;
}

RARetCode CentralLooper::enqueueTask(ITask* task)
{
	if (!task) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}

	std::lock_guard<std::mutex> lock(mMutex);
	mTaskQueue.push(task);
	return RARetCode::RET_OK;
}

RARetCode CentralLooper::registerFrameSyncCallback(IFrameSyncCallback* cb)
{
	if (!cb) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}

	std::lock_guard<std::mutex> lock(mMutex);
	mFrameSyncCallbacks.push_back(cb);
	return RARetCode::RET_OK;
	;
}

RARetCode CentralLooper::unregisterFrameSyncCallback(IFrameSyncCallback* cb)
{
	if (!cb) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}

	std::lock_guard<std::mutex> lock(mMutex);

	if (0 != Utility::eraseVectorElm(mFrameSyncCallbacks, cb)) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}

	return RARetCode::RET_OK;
}

CentralLooper::CentralLooper() :
	mStarted(false), mTimeOfFrame(0)
{
}

////////////////////////////////////
// Private
////////////////////////////////////
void CentralLooper::run() {
	while (mStarted) {
		auto frameStart = std::chrono::steady_clock::now();
		while (std::chrono::steady_clock::now() - frameStart < std::chrono::milliseconds(mTimeOfFrame)) {
			ITask* task = dequeue();
			if (task) {
				auto taskStart = std::chrono::steady_clock::now();
				task->doTask();
				task->finish();
			}
			else {
				std::this_thread::yield();
			}
		}

		for (IFrameSyncCallback* frameSyncCallback : mFrameSyncCallbacks) {
			frameSyncCallback->sync();
		}

		auto takenTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - frameStart);
		int takenTimeInt = static_cast<int>(takenTime.count() / 1000);
		if (takenTimeInt > mTimeOfFrame) {
			Utility::printLog("The frame time is over specified time.(%d)", takenTime);
		}
	}
}

CentralLooper::ITask* CentralLooper::dequeue() {
	ITask* task = NULL;

	std::lock_guard<std::mutex> lock(mMutex);

	if (!mTaskQueue.empty()){
		task = mTaskQueue.front();
		mTaskQueue.pop();
	}

	return task;
}

