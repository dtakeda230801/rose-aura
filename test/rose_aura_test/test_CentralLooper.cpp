#include "pch.h"
#include "rose_aura_test.h"
#include "RoseAura.h"

class TestTask : public ICentralLooper::ITask {
public:
	int	doTask()
	{
		return 0;
	};

	int	finish()
	{
		return 0;
	};

	std::string getTaskId()
	{
		return "Test Task";
	};

	virtual ~TestTask() = default;
	TestTask() = default;
};

class TestCallback : public ICentralLooper::IFrameSyncCallback {
public:
	int sync()
	{
		return 0;
	};

	virtual ~TestCallback() = default;
	TestCallback() = default;
};

TEST(testCentralLooper, APITest)
{
	ROSE_AURA_TEST_BEGIN;
	{
		int ret;

		TestTask*		testTask = new TestTask();
		TestCallback*	testCb	 = new TestCallback();

		std::unique_ptr<RoseAura> ra = RoseAura::create();

		ICentralLooper& cl = ra->getCentralLooper();

		cl.enqueueTask(testTask);
		cl.registerFrameSyncCallback(testCb);
		cl.unregisterFrameSyncCallback(testCb);

		delete testTask;
		delete testCb;
	}
	ROSE_AURA_TEST_FIN;
}