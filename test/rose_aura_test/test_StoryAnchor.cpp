#include "pch.h"
#include "rose_aura_test.h"
#include "RoseAura.h"

#include "Utility.h"

class StoryAnchorCallback : public IStoryAnchor::IStoryPointCallback
{
public:
	void onStateChanged(std::string storyPoint, IStoryAnchor::StoryPointState state)
	{
		Utility::printLog("onStateChanged:%s -> %d", storyPoint.c_str(), state);
	}

	StoryAnchorCallback() = default;
	virtual ~StoryAnchorCallback() = default;
};



TEST(testStoryAnchor, APITest)
{
	ROSE_AURA_TEST_BEGIN;
	{
		std::vector<std::string> spList;

		StoryAnchorCallback* saCb = new StoryAnchorCallback();


		std::unique_ptr<RoseAura> ra = RoseAura::create();
		IStoryAnchor& sa = ra->getStoryAnchor();

		//////////////////////
		EXPECT_EQ(sa.registerStoryPointCallback(nullptr), RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(sa.registerStoryPointCallback(saCb), RARetCode::RET_OK);
		EXPECT_EQ(sa.unregisterStoryPointCallback(nullptr), RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(sa.unregisterStoryPointCallback(saCb), RARetCode::RET_OK);

		EXPECT_EQ(sa.registerStoryPointCallback(saCb), RARetCode::RET_OK);

		//////////////////////
		EXPECT_EQ(sa.loadStoryGraph("..\\..\\test\\rose_aura_test\\testStoryAnchor.json"), RARetCode::RET_OK);

		spList.clear();
		spList = sa.getStoryPointsByState(IStoryAnchor::StoryPointState::HIDDEN);
		for (auto& name : spList) {
			Utility::printLog("sp:%s", name.c_str());
		}

		Utility::printLog("---");

		EXPECT_EQ(sa.changeState("Test Story Point A", IStoryAnchor::StoryPointState::AVAILABLE), RARetCode::RET_OK);

		Utility::printLog("---");

		spList.clear();
		spList = sa.getStoryPointsByState(IStoryAnchor::StoryPointState::AVAILABLE);
		for (auto& name : spList) {
			Utility::printLog("sp:%s", name.c_str());
		}

		Utility::printLog("---");

		EXPECT_EQ(sa.changeState("Test Story Point F", IStoryAnchor::StoryPointState::AVAILABLE), RARetCode::RET_ERR_INVALID_STATE);

		EXPECT_EQ(sa.changeState("Test Story Point A", IStoryAnchor::StoryPointState::COMPLETED), RARetCode::RET_OK);

		EXPECT_EQ(sa.changeState("Test Story Point F", IStoryAnchor::StoryPointState::AVAILABLE), RARetCode::RET_OK);

		EXPECT_EQ(sa.changeState("Test Story Point F", IStoryAnchor::StoryPointState::DISABLED), RARetCode::RET_ERR_INVALID_STATE);

		Utility::printLog("---");
		EXPECT_EQ(sa.changeState("Test Story Point B", IStoryAnchor::StoryPointState::COMPLETED), RARetCode::RET_OK);
		EXPECT_EQ(sa.changeState("Test Story Point C", IStoryAnchor::StoryPointState::COMPLETED), RARetCode::RET_OK);

		EXPECT_EQ(sa.changeState("Test Story Point F", IStoryAnchor::StoryPointState::DISABLED), RARetCode::RET_OK);

		Utility::printLog("---");
		spList.clear();
		spList = sa.getStoryPointsByState(IStoryAnchor::StoryPointState::DISABLED);
		for (auto& name : spList) {
			Utility::printLog("sp:%s", name.c_str());
		}


		delete saCb;
	}
	ROSE_AURA_TEST_FIN;
}