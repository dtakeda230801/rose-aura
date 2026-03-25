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
	int 		 setCurrentWorld(WORLD_ID id);
	WORLD_ID	 changeWorld(WORLD_ID id);
	ActiveSpace& getActiveSpace();
	int 		 setActiveSpace(ActiveSpace& space);
	int          updatePosition(Vec3& center);
	Vec3&		 getPosition();

	int			 registerEvent(TRIGGER_ID trigger, Vec3& location, float distance);
	int			 removeEvent(TRIGGER_ID trigger);

	void		 registerApproachingCallback(IApproachingCallback* approachingCallback);
	void		 unregisterApproachingCallback(IApproachingCallback* approachingCallback);
	void		 registerTriggerCallback(ITriggerCallback* triggerCallback);
	void		 unregisterTriggerCallback(ITriggerCallback* triggerCallback);
	void		 registerActiveSpaceUpdate(IActiveSpaceCallback* activeSpaceCallback);
	void		 unregisterActiveSpaceUpdate(IActiveSpaceCallback* activeSpaceCallback);

	WorldNavigator() = default;
	virtual ~WorldNavigator() = default;
private:
	struct World {
		WORLD_ID	mId;
		WorldConfig mWorldCongig;
		Vec3		mPosition;
	};

	World				mCurrentWorld;

	std::vector<World>	mWorlds;


	std::vector<IApproachingCallback*>	mApproachingCallbacks;
	std::vector<ITriggerCallback*>		mTriggerCallbacks;
	std::vector<IActiveSpaceCallback*>	mActiveSpaceCallbacks;
};