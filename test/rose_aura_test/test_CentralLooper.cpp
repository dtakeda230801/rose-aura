#include "pch.h"
#include "rose_aura_test.h"

#include "RoseAura.h"
#include "RoseAuraReturnCode.h"

using namespace RoseAuraReturnCode;

class TestTask : public ICentralLooper::ITask {
public:
	void doTask()
	{
	};

	void finish()
	{
	};

	std::string getTaskName()
	{
		return "Test Task";
	};

	virtual ~TestTask() = default;
	TestTask() = default;
};

class TestCallback : public ICentralLooper::IFrameSyncCallback {
public:
	void sync()
	{
	};

	virtual ~TestCallback() = default;
	TestCallback() = default;
};

TEST(testCentralLooper, APITest)
{
	ROSE_AURA_TEST_BEGIN;
	{
		TestTask*		testTask = new TestTask();
		TestCallback*	testCb	 = new TestCallback();
		TestCallback*   testCb2  = new TestCallback();

		std::unique_ptr<RoseAura> ra = RoseAura::create();

		ICentralLooper& cl = ra->getCentralLooper();

		EXPECT_EQ(cl.enqueueTask(nullptr)					, RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(cl.enqueueTask(testTask)					, RARetCode::RET_OK);
		EXPECT_EQ(cl.registerFrameSyncCallback(nullptr)		, RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(cl.registerFrameSyncCallback(testCb)		, RARetCode::RET_OK);
		EXPECT_EQ(cl.unregisterFrameSyncCallback(nullptr)   , RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(cl.unregisterFrameSyncCallback(testCb2)   , RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(cl.unregisterFrameSyncCallback(testCb)	, RARetCode::RET_OK);

		EXPECT_EQ(cl.stop()		, RARetCode::RET_ERR_INVALID_STATE);
		EXPECT_EQ(cl.start(100)	, RARetCode::RET_OK);
		EXPECT_EQ(cl.start(100)	, RARetCode::RET_ERR_INVALID_STATE);
		EXPECT_EQ(cl.stop()		, RARetCode::RET_OK);;

		delete testTask;
		delete testCb;
		delete testCb2;
	}
	ROSE_AURA_TEST_FIN;
}