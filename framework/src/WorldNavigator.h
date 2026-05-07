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
	RARetCode    movePrimaryEntityPosition(Vec3& pos);
	Vec3		 getPrimaryEntityPosition();

	RARetCode	 registerCollisionCallback(ICollisionCallback* cb);
	RARetCode	 unregisterCollisionCallback();

	RARetCode	 registerEntity(ENTITY_ID id, Vec3& location, float distance, ICollisionCallback* cb);
	RARetCode	 removeEntity(ENTITY_ID id);
	RARetCode	 moveEntity(ENTITY_ID id, Vec3& location);
	RARetCode    getEntityLocation(ENTITY_ID id, Vec3* location);

	RARetCode	 registerActiveSpaceCallback(IActiveSpaceCallback* cb);
	RARetCode	 unregisterActiveSpaceCallback();

	WorldNavigator();
	virtual ~WorldNavigator() = default;
private:
	struct Entity {
		ENTITY_ID	mId;
		Vec3		mLocation;
		float		mDistance;

		ICollisionCallback*	mCb;
	};

	Vec3     calcCenter(Bounds& bounds);
	Bounds   calcBounds(Vec3& center, Extent& range);
	bool     fitWithin(Bounds& base, Bounds& target);
	bool     fitWithin(Bounds& base, Vec3& target);
	Vec3     adjustPosition(Bounds& base, Vec3& pos);
	int      selectBoundaryPosition(int max, int min, int val);
	Entity* findEntity(ENTITY_ID id);

	struct World {
		WORLD_ID	mId;
		Bounds      mWorldSpace;
		Extent      mActiveRange;
		Bounds      mActiveSpace;
		Extent		mNonScrollRange;
		Bounds      mNonScrollSpace;
		bool        mEnableFollowing;
		bool		mLimitScrolling;
		Vec3		mPrimaryEntityPosition;
		Vec3        mScrollPosition;

		IActiveSpaceCallback*
					mActiveSpaceCb;

		ICollisionCallback*
					mCollisionCb;
		std::vector<Entity>
			mEntities;
	};

	std::mutex      mMutex;

	std::vector<World>
					mWorlds;
	int 			mCurrentWorldIndex;
};