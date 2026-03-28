#pragma once

#include <vector>
#include <mutex>

#include "IWorldNavigator.h"

class WorldNavigator : public IWorldNavigator 
{
public:
	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	WORLD_ID	 createWorld(WorldConfig& space);
	WORLD_ID	 getCurrentWorld();
	RARetCode	 changeWorld(WORLD_ID id);
	Bounds		 getActiveSpace();
	Vec3         getActiveSpacePosition();
	RARetCode    moveActiveSpace(Vec3& pos);
	void		 resetActiveSpace();
	RARetCode    movePosition(Vec3& pos);
	Vec3		 getPosition();

	RARetCode	 registerTrigger(TRIGGER_ID id, Vec3& location, float distance);
	RARetCode	 removeTrigger(TRIGGER_ID id);

	RARetCode	 registerTriggerCallback(ITriggerCallback* cb);
	RARetCode	 unregisterTriggerCallback(ITriggerCallback* cb);
	RARetCode	 registerActiveSpaceUpdate(IActiveSpaceCallback* cb);
	RARetCode	 unregisterActiveSpaceUpdate(IActiveSpaceCallback* cb);

	WorldNavigator();
	virtual ~WorldNavigator() = default;
private:
	float  calcDistance(int a, int b);
	Vec3   calcCenter(Bounds& bounds);
	Bounds calcBounds(Vec3& center, Extent& range);
	bool   fitWithin(Bounds& base, Bounds& target);
	bool   fitWithin(Bounds& base, Vec3& target);
	Vec3   adjustPosition(Bounds& base, Vec3& current, Vec3& next);
	int    checkCrossing(int max, int min, int current, int next);

	struct Trigger {
		TRIGGER_ID	mId;
		Vec3		mLocation;
		float		mDistance;
	};

	struct World {
		WORLD_ID	mId;
		Bounds      mWorldSpace;
		Extent      mActiveRange;
		Bounds      mActiveSpace;
		Extent		mNonScrollRange;
		Bounds      mNonScrollSpace;
		bool        mEnableFollowing;
		bool		mLimitScrolling;
		Vec3		mPosition;
		Vec3        mScrollPosition;

		std::vector<Trigger>
					mTriggers;
	};

	std::mutex      mMutex;

	std::vector<World>		mWorlds;
	World*					mCurrentWorld;

	std::vector<ITriggerCallback*>		mTriggerCallbacks;
	std::vector<IActiveSpaceCallback*>	mActiveSpaceCallbacks;
};