#include "pch.h"
#include "rose_aura_test.h"

#include "RoseAura.h"
#include "RoseAuraReturnCode.h"

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

	conf1.mPrimaryEntityPosition.mX = 0;
	conf1.mPrimaryEntityPosition.mY = 0;
	conf1.mPrimaryEntityPosition.mZ = 0;

	conf1.mEnableFollowing = true;
	conf1.mLimitScrolling  = true;
}

class EntityCallback : public IWorldNavigator::ICollisionCallback
{
public:
	ICollisionCallback::CollisionResult onApproaching(
		               IWorldNavigator::WORLD_ID	worldId
		             , IWorldNavigator::ENTITY_ID	entityId
					 , IWorldNavigator::Vec3		from
		             , IWorldNavigator::Vec3&		to)
	{
		Utility::printLog("[EntityCallback::onApproaching] World:%d Entity:%d", worldId, entityId);
		mRevApproaching = true;

		if (mReturn == CollisionResult::INHIBITED) {
			to = mUpdate;
		}

		return mReturn;
	}
	void onHit(IWorldNavigator::WORLD_ID  worldId
		     , IWorldNavigator::ENTITY_ID entityId)
	{
		Utility::printLog("[EntityCallback::onHit] World:%d Entity:%d", worldId, entityId);
		mRevHit = true;
	}

	void setBehaiviorWithReset(CollisionResult ret, IWorldNavigator::Vec3 update)
	{
		mReturn = ret;
		mUpdate = update;
		mRevApproaching = false;
		mRevHit         = false;
	}

	bool checkResult(bool approaching, bool hit)
	{
		return (approaching == mRevApproaching && hit == mRevHit);
	}

	EntityCallback()
		: mRevApproaching(false)
		, mRevHit(false)
		, mReturn(CollisionResult::NO_COLLISION)
		, mUpdate{}
	{
	}

	virtual ~EntityCallback() = default;

private:
	bool	mRevApproaching;
	bool    mRevHit;

	CollisionResult			 mReturn;
	IWorldNavigator::Vec3    mUpdate;
};

class ActiveSpaceCallback : public IWorldNavigator::IActiveSpaceCallback
{
public:
	void onUpdate(IWorldNavigator::WORLD_ID worldId
		        , IWorldNavigator::Bounds   activeSpace)
	{
		Utility::printLog("[ActiveSpaceCallback::onUpdate] World:%d", worldId);
		mRevUpdate = true;
	}

	void reset()
	{
		mRevUpdate = false;
	}

	bool checkResult(bool update)
	{
		return (mRevUpdate == update);
	}

	ActiveSpaceCallback()
		: mRevUpdate(false)
	{
	}

	virtual ~ActiveSpaceCallback() = default;
private:
	bool mRevUpdate;
};

#define SET_VEC(vec,x,y,z)		vec.mX=x;vec.mY=y;vec.mZ=z; 
#define CHECK_VEC(vec,x,y,z)	(vec.mX==x&&vec.mY==y&&vec.mZ==z) 

TEST(testWorldNavigator, APITest)
{
	ROSE_AURA_TEST_BEGIN;
	{
		ActiveSpaceCallback* activeSpaceCb   = new ActiveSpaceCallback();
		EntityCallback*      entityCb1       = new EntityCallback();
		EntityCallback*		 entityCb2  	 = new EntityCallback();
		EntityCallback*      primaryEntityCb = new EntityCallback();

		buildConf1();

		IWorldNavigator::WORLD_ID w1, w2;
		IWorldNavigator::Bounds   b;
		IWorldNavigator::Vec3	  v;
		IWorldNavigator::Vec3	  vResult;

		///////////////////////////////////////////
		std::unique_ptr<RoseAura> ra = RoseAura::create();

		IWorldNavigator& wn = ra->getWorldNavigator();

		w1 = wn.createWorld(conf1);
		EXPECT_TRUE(isValidWorldId(w1));
		
		EXPECT_EQ(wn.registerActiveSpaceCallback(nullptr), RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(wn.registerActiveSpaceCallback(activeSpaceCb), RARetCode::RET_OK);
		EXPECT_EQ(wn.registerActiveSpaceCallback(activeSpaceCb), RARetCode::RET_ERR_INVALID_STATE);
		EXPECT_EQ(wn.unregisterActiveSpaceCallback()  , RARetCode::RET_OK);
		EXPECT_EQ(wn.unregisterActiveSpaceCallback()  , RARetCode::RET_ERR_INVALID_STATE);
		EXPECT_EQ(wn.registerActiveSpaceCallback(activeSpaceCb), RARetCode::RET_OK);

		EXPECT_EQ(wn.registerCollisionCallback(nullptr), RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(wn.registerCollisionCallback(primaryEntityCb), RARetCode::RET_OK);
		EXPECT_EQ(wn.registerCollisionCallback(primaryEntityCb), RARetCode::RET_ERR_INVALID_STATE);
		EXPECT_EQ(wn.unregisterCollisionCallback(), RARetCode::RET_OK);
		EXPECT_EQ(wn.unregisterCollisionCallback(), RARetCode::RET_ERR_INVALID_STATE);
		EXPECT_EQ(wn.registerCollisionCallback(primaryEntityCb), RARetCode::RET_OK);

		w2 = wn.createWorld(conf1);
		EXPECT_TRUE(isValidWorldId(w2));

		EXPECT_EQ(wn.getCurrentWorld(), w1);

		EXPECT_EQ(wn.changeWorld(10)  , RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(wn.changeWorld(w2)  , RARetCode::RET_OK);
		EXPECT_EQ(wn.getCurrentWorld(), w2);
		EXPECT_EQ(wn.changeWorld(w1)  , RARetCode::RET_OK);

		b = wn.getActiveSpace();
		EXPECT_TRUE(CHECK_VEC(b.mMax,50,50,50) && CHECK_VEC(b.mMin, -50, -50, -50));

		v = wn.getActiveSpacePosition();
		EXPECT_TRUE(CHECK_VEC(v, 0, 0, 0));

		///////////////////////////////////////////
		activeSpaceCb->reset();

		SET_VEC(v,10,10,10);
		EXPECT_EQ(wn.moveActiveSpace(v), RARetCode::RET_OK);

		b = wn.getActiveSpace();
		EXPECT_TRUE(CHECK_VEC(b.mMax, 60, 60, 60) && CHECK_VEC(b.mMin, -40, -40, -40));

		v = wn.getActiveSpacePosition();
		EXPECT_TRUE(CHECK_VEC(v, 10, 10, 10));

		EXPECT_TRUE(activeSpaceCb->checkResult(true));

		///////////////////////////////////////////
		wn.resetActiveSpace();
		b = wn.getActiveSpace();
		EXPECT_TRUE(CHECK_VEC(b.mMax, 50, 50, 50) && CHECK_VEC(b.mMin, -50, -50, -50));

		v = wn.getActiveSpacePosition();
		EXPECT_TRUE(CHECK_VEC(v, 0, 0, 0));

		///////////////////////////////////////////
		SET_VEC(v, 10, 10, 10);
		EXPECT_EQ(wn.movePrimaryEntityPosition(v), RARetCode::RET_OK);
		v = wn.getPrimaryEntityPosition();
		EXPECT_TRUE(CHECK_VEC(v, 10, 10, 10));

		SET_VEC(v, 500, 500, 500);
		EXPECT_EQ(wn.movePrimaryEntityPosition(v), RARetCode::RET_ADJUSTED);
		v = wn.getPrimaryEntityPosition();
		EXPECT_TRUE(CHECK_VEC(v, 200, 200, 200));

		///////////////////////////////////////////
		SET_VEC(v, 0, 0, 0);
		EXPECT_EQ(wn.movePrimaryEntityPosition(v), RARetCode::RET_OK);

		SET_VEC(v, 100, 100, 100);
		EXPECT_EQ(wn.registerEntity(0x0001, v, 30.0f, nullptr)    , RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(wn.registerEntity(0x0001, v, 30.0f, entityCb1)   , RARetCode::RET_OK);
		EXPECT_EQ(wn.registerEntity(0x0001, v, 30.0f, entityCb1)   , RARetCode::RET_ERR_INVALID_PARAMS);

		SET_VEC(v, 150, 150, 150);

		EXPECT_EQ(wn.moveEntity(0x0001, v), RARetCode::RET_OK);
		EXPECT_EQ(wn.moveEntity(0x0002, v), RARetCode::RET_ERR_INVALID_ARG);

		EXPECT_EQ(wn.getEntityLocation(0x0001, &vResult), RARetCode::RET_OK);
		EXPECT_TRUE(CHECK_VEC(vResult, 150, 150, 150));

		SET_VEC(v, 300, 300, 300);
		EXPECT_EQ(wn.moveEntity(0x0001, v), RARetCode::RET_ADJUSTED);

		EXPECT_EQ(wn.getEntityLocation(0x0002, &vResult), RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(wn.getEntityLocation(0x0001, nullptr) , RARetCode::RET_ERR_INVALID_ARG);

		///////////////////////////////////////////
		entityCb1->setBehaiviorWithReset(IWorldNavigator::ICollisionCallback::CollisionResult::NO_COLLISION, v);
		SET_VEC(v, 3, 3, 3);
		EXPECT_EQ(wn.moveEntity(0x0001, v), RARetCode::RET_OK);
		EXPECT_TRUE(entityCb1->checkResult(true,false));

		entityCb1->setBehaiviorWithReset(IWorldNavigator::ICollisionCallback::CollisionResult::HIT, v);
		SET_VEC(v, 2, 2, 2);
		EXPECT_EQ(wn.moveEntity(0x0001, v), RARetCode::RET_OK);
		EXPECT_TRUE(entityCb1->checkResult(true, true));

		SET_VEC(v, 2, 2, 2);
		primaryEntityCb->setBehaiviorWithReset(IWorldNavigator::ICollisionCallback::CollisionResult::INHIBITED, v);
		SET_VEC(v, 1, 1, 1);
		EXPECT_EQ(wn.moveEntity(0x0001, v), RARetCode::RET_ADJUSTED);
		EXPECT_TRUE(primaryEntityCb->checkResult(true, false));

		EXPECT_TRUE(primaryEntityCb->checkResult(true, false));
		EXPECT_EQ(wn.getEntityLocation(0x0001, &vResult), RARetCode::RET_OK);
		EXPECT_TRUE(CHECK_VEC(vResult, 2, 2, 2));

		///////////////////////////////////////////
		SET_VEC(v, 10, 10, 10);
		EXPECT_EQ(wn.moveEntity(0x0001, v), RARetCode::RET_ADJUSTED);

		SET_VEC(v, 100, 100, 100);
		EXPECT_EQ(wn.registerEntity(0x0002, v, 30.0f, entityCb2), RARetCode::RET_OK);

		SET_VEC(v, 90, 90, 90);
		entityCb2->setBehaiviorWithReset(IWorldNavigator::ICollisionCallback::CollisionResult::INHIBITED, v);
		SET_VEC(v, 95, 95, 95);
		EXPECT_EQ(wn.moveEntity(0x0001, v), RARetCode::RET_ADJUSTED);
		EXPECT_TRUE(entityCb2->checkResult(true, false));

		EXPECT_EQ(wn.getEntityLocation(0x0001, &vResult), RARetCode::RET_OK);
		EXPECT_TRUE(CHECK_VEC(vResult, 90, 90, 90));

		///////////////////////////////////////////
		EXPECT_EQ(wn.removeEntity(0x0001), RARetCode::RET_OK);
		EXPECT_EQ(wn.removeEntity(0x0001), RARetCode::RET_ERR_INVALID_ARG);

		///////////////////////////////////////////
		delete primaryEntityCb;
		delete entityCb2;
		delete entityCb1;
		delete activeSpaceCb;
	}
	ROSE_AURA_TEST_FIN;
}