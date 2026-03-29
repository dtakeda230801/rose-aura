#include "pch.h"
#include "rose_aura_test.h"

#include "RoseAura.h"
#include "RoseAuraReturnCode.h"

using namespace RoseAuraReturnCode;

IWorldNavigator::WorldConfig conf1;

void buildConf1()
{
	conf1.mWorldSpace.mMax.mX =  200;
	conf1.mWorldSpace.mMax.mY =  200;
	conf1.mWorldSpace.mMax.mZ =  200;
	conf1.mWorldSpace.mMin.mX = -200;
	conf1.mWorldSpace.mMin.mY = -200;
	conf1.mWorldSpace.mMin.mZ = -200;

	conf1.mActiveRange.mX = 50;
	conf1.mActiveRange.mY = 50;
	conf1.mActiveRange.mZ = 50;

	conf1.mNonScrollRange.mX = 20;
	conf1.mNonScrollRange.mY = 20;
	conf1.mNonScrollRange.mZ = 20;

	conf1.mPosition.mX = 0;
	conf1.mPosition.mY = 0;
	conf1.mPosition.mZ = 0;

	conf1.mEnableFollowing = true;
	conf1.mLimitScrolling  = true;
}

class TriggerCallback : public IWorldNavigator::ITriggerCallback
{
public:
	bool onApproaching(IWorldNavigator::WORLD_ID worldId
		             , IWorldNavigator::TRIGGER_ID eventId
		             , IWorldNavigator::Vec3& position)
	{
		return false;
	}
	void onTrigger(IWorldNavigator::WORLD_ID worldId
		         , IWorldNavigator::TRIGGER_ID eventId)
	{
	}
	TriggerCallback() = default;
	virtual ~TriggerCallback() = default;
};

class ActiveSpaceCallback : public IWorldNavigator::IActiveSpaceCallback
{
public:
	void onUpdate(IWorldNavigator::WORLD_ID worldId
		, IWorldNavigator::Bounds activeSpace)
	{
	}

	ActiveSpaceCallback() = default;
	virtual ~ActiveSpaceCallback() = default;
};
TEST(testWorldNavigator, APITest)
{
	ROSE_AURA_TEST_BEGIN;
	{
		ActiveSpaceCallback* asCb = new ActiveSpaceCallback();
		TriggerCallback*     trCb = new TriggerCallback();

		buildConf1();

		bool check;
		IWorldNavigator::WORLD_ID w1, w2;
		IWorldNavigator::Bounds   b;
		IWorldNavigator::Vec3	  v;

		///////////////////////////////////////////
		std::unique_ptr<RoseAura> ra = RoseAura::create();

		IWorldNavigator& wn = ra->getWorldNavigator();

		conf1.mActiveSpaceCb = nullptr;
		w1 = wn.createWorld(conf1);
		EXPECT_FALSE(isValidWorldId(w1));

		conf1.mActiveSpaceCb = asCb;
		w1 = wn.createWorld(conf1);
		EXPECT_TRUE(isValidWorldId(w1));

		w2 = wn.createWorld(conf1);
		EXPECT_TRUE(isValidWorldId(w2));

		EXPECT_EQ(wn.getCurrentWorld(), w1);

		EXPECT_EQ(wn.changeWorld(10)  , RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(wn.changeWorld(w2)  , RARetCode::RET_OK);
		EXPECT_EQ(wn.getCurrentWorld(), w2);
		EXPECT_EQ(wn.changeWorld(w1)  , RARetCode::RET_OK);

		b = wn.getActiveSpace();
		check = (b.mMax.mX ==  50) && (b.mMax.mY ==  50) && (b.mMax.mZ ==  50)
			 && (b.mMin.mX == -50) && (b.mMin.mY == -50) && (b.mMin.mZ == -50);
		EXPECT_TRUE(check);

		v = wn.getActiveSpacePosition();
		check = (v.mX == 0) && (v.mY == 0) && (v.mZ == 0);
		EXPECT_TRUE(check);

		///////////////////////////////////////////
		v.mX = 10;
		v.mY = 10;
		v.mZ = 10;
		EXPECT_EQ(wn.moveActiveSpace(v), RARetCode::RET_OK);

		b = wn.getActiveSpace();
		check = (b.mMax.mX == 60) && (b.mMax.mY == 60) && (b.mMax.mZ == 60)
			&& (b.mMin.mX == -40) && (b.mMin.mY == -40) && (b.mMin.mZ == -40);
		EXPECT_TRUE(check);

		v = wn.getActiveSpacePosition();
		check = (v.mX == 10) && (v.mY == 10) && (v.mZ == 10);
		EXPECT_TRUE(check);

		///////////////////////////////////////////
		wn.resetActiveSpace();
		b = wn.getActiveSpace();
		check = (b.mMax.mX == 50) && (b.mMax.mY == 50) && (b.mMax.mZ == 50)
			&& (b.mMin.mX == -50) && (b.mMin.mY == -50) && (b.mMin.mZ == -50);
		EXPECT_TRUE(check);

		v = wn.getActiveSpacePosition();
		check = (v.mX == 0) && (v.mY == 0) && (v.mZ == 0);
		EXPECT_TRUE(check);

		///////////////////////////////////////////
		v.mX = 10;
		v.mY = 10;
		v.mZ = 10;
		EXPECT_EQ(wn.movePosition(v), RARetCode::RET_OK);
		v = wn.getPosition();
		check = (v.mX == 10) && (v.mY == 10) && (v.mZ == 10);
		EXPECT_TRUE(check);

		v.mX = 500;
		v.mY = 500;
		v.mZ = 500;
		EXPECT_EQ(wn.movePosition(v), RARetCode::RET_ADJUSTED);
		v = wn.getPosition();
		check = (v.mX == 200) && (v.mY == 200) && (v.mZ == 200);
		EXPECT_TRUE(check);
		///////////////////////////////////////////
		v.mX = 100;
		v.mY = 100;
		v.mZ = 100;
		EXPECT_EQ(wn.registerTrigger(0x0001, v, 30.0f, nullptr), RARetCode::RET_ERR_INVALID_PARAMS);
		EXPECT_EQ(wn.registerTrigger(0x0001, v, 30.0f, trCb)   , RARetCode::RET_OK);
		EXPECT_EQ(wn.registerTrigger(0x0001, v, 30.0f, trCb)   , RARetCode::RET_ERR_INVALID_PARAMS);
		EXPECT_EQ(wn.removeTrigger(0x0001), RARetCode::RET_OK);
		EXPECT_EQ(wn.removeTrigger(0x0001), RARetCode::RET_ERR_INVALID_ARG);

		///////////////////////////////////////////
		delete trCb;
		delete asCb;
	}
	ROSE_AURA_TEST_FIN;
}