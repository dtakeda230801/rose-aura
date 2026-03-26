#pragma once

#include <vector>

#include "IWorldNavigator.h"

class WorldNavigator : public IWorldNavigator 
{
public:
	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	WORLD_ID	 createWorld(WorldConfig& space);
	WORLD_ID	 getCurrentWorld();
	WORLD_ID	 changeWorld(WORLD_ID id);
	ActiveSpace& getActiveSpace();
	int 		 setActiveSpace(ActiveSpace& space);
	int          updatePosition(Vec3& pos);
	Vec3&		 getPosition();

	int			 registerTrigger(TRIGGER_ID id, Vec3& location, float distance);
	int			 removeTrigger(TRIGGER_ID id);

	void		 registerApproachingCallback(IApproachingCallback* cb);
	void		 unregisterApproachingCallback(IApproachingCallback* cb);
	void		 registerTriggerCallback(ITriggerCallback* cb);
	void		 unregisterTriggerCallback(ITriggerCallback* cb);
	void		 registerActiveSpaceUpdate(IActiveSpaceCallback* cb);
	void		 unregisterActiveSpaceUpdate(IActiveSpaceCallback* cb);

	WorldNavigator();
	virtual ~WorldNavigator() = default;
private:
	struct World {
		WORLD_ID	mId;
		WorldConfig mWorldCongig;
		Vec3		mPosition;
	};

	struct Trigger {
		TRIGGER_ID	mId;
		Vec3		mLocation;
		float		mDistance;
	};

	World*					mCurrentWorld;

	std::vector<World>		mWorlds;
	std::vector<Trigger>	mTriggers;

	std::vector<IApproachingCallback*>	mApproachingCallbacks;
	std::vector<ITriggerCallback*>		mTriggerCallbacks;
	std::vector<IActiveSpaceCallback*>	mActiveSpaceCallbacks;
};