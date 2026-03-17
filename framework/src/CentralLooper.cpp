#include "CentralLooper.h"
#include <cstddef>
#include <memory>

////////////////////////////////////
// APIs
////////////////////////////////////
void CentralLooper::createInstance()
{
	if (!mInstance) {
		mInstance = std::unique_ptr<CentralLooper>(new CentralLooper());
	}
}

void CentralLooper::destroyInstance() {
	if (mInstance) {
		mInstance.reset();
	}
}

CentralLooper& CentralLooper::getInstance() {
	return *mInstance;
}

int CentralLooper::start(int timeOfFrame) {
	if (mStarted) {
		return -1;
	}

	//TODO check timeOfFrame

	mTimeOfFrame = timeOfFrame;

	mStarted = true;

	mThread = std::thread(&CentralLooper::run, this);
	return 0;
}

int CentralLooper::stop() {
	if (!mStarted) {
		return -1;
	}
	mStarted = false;

	if (mThread.joinable()) {
		mThread.join();
	}
	return 0;
}

int CentralLooper::setTask(ITask* task) {
	if (!task) {
		return -1;
	}

	std::lock_guard<std::mutex> lock(mMutex);
	mTaskQueue.push(task);
	return 0;
}

int CentralLooper::registerFrameSyncCallback(IFrameSyncCallback* cb) {
	mFrameSyncCallbacks.push_back(cb);
	return 0;
}


////////////////////////////////////
// Private
////////////////////////////////////
std::unique_ptr<CentralLooper> CentralLooper::mInstance = nullptr;

CentralLooper::CentralLooper() :
	mStarted(false),mTimeOfFrame(0)
{
}

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

		auto TimeTaken = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - frameStart);
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

