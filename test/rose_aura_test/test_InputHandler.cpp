#include "pch.h"
#include "rose_aura_test.h"

#include "RoseAura.h"
#include "RoseAuraReturnCode.h"

class TestCallback : public IInputHandler::IInputHandlerCallback {
public:
	void onInputEvent(std::vector<std::pair<InputState, InputType>>& events)
	{
	}

	virtual ~TestCallback() = default;
	TestCallback() = default;
};

std::string TestJson = "{ \"UP\": { \"xinput\": \"0x0001\", \"keyboard\": \"W\" } }";

TEST(testInputHandler, APITest)
{
	ROSE_AURA_TEST_BEGIN;
	{
		TestCallback* testCb = new TestCallback();
		TestCallback* testCb2 = new TestCallback();

		std::unique_ptr<RoseAura> ra = RoseAura::create();

		IInputHandler& ih = ra->getInputHandler();

		EXPECT_EQ(ih.registerCallback(nullptr)   , RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(ih.registerCallback(testCb)    , RARetCode::RET_OK);
		EXPECT_EQ(ih.unregisterCallback(nullptr) , RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(ih.unregisterCallback(testCb2) , RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(ih.unregisterCallback(testCb)  , RARetCode::RET_OK);

		EXPECT_EQ(ih.setConf("test")             , RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(ih.setConf(TestJson)        	 , RARetCode::RET_OK);

		void update();

		delete testCb;
		delete testCb2;
	}
	ROSE_AURA_TEST_FIN;
}