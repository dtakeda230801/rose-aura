#include "GraphicsManager.h"
#include "CentralLooper.h"
#include "Utility.h"

class LocalTask : public CentralLooper::ITask {
public:
	int doTask() {
		Utility::printLog("do Task");
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		return 0;
	};
	int finish() {
		Utility::printLog("finish");
		return 0;
	};
	std::string getTaskId() {
		return "Test Task";
	}
	
	LocalTask()  = default;
	~LocalTask() = default;
};

class FrameSync : public CentralLooper::IFrameSyncCallback {
public:
	FrameSync(LocalTask* task) : mLocalTask(task), mCount(0) {

	}
	int sync() {
		mCount++;
		Utility::printLog("frame sync(%d)", mCount);
		CentralLooper::getInstance()->setTask(mLocalTask);
		return 0;
	}

	FrameSync()  = default;
	~FrameSync() = default;

private:
	LocalTask* mLocalTask;
	int		   mCount;
};

int main() {
	Utility::printLog("TEST");

	CentralLooper* cp = CentralLooper::getInstance();

	LocalTask* localTask = new LocalTask();
	FrameSync* fsync = new FrameSync(localTask);

	cp->registerFrameSyncCallback(fsync);
	cp->start(30);

	GraphicsManager* gMgr = GraphicsManager::getInstance();
	gMgr->run();
	GraphicsManager::destroyInstance();

	cp->stop();

	CentralLooper::destroyInstance();
	delete localTask;
	delete fsync;

	_CrtDumpMemoryLeaks();
	return 0;
}