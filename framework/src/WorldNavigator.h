#pragma once

#include "IWorldNavigator.h"

class WorldNavigator : public IWorldNavigator 
{
public:
	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	WORLD_ID	 createWorld(SpaceConfig& space);
	WORLD_ID	 getCurrentWorld();
	int 		 setCurrentWorld(WORLD_ID id);
	WORLD_ID	 changeWorld(WORLD_ID id);
	ActiveSpace& getActiveSpace();
	int 		 setActiveSpace(ActiveSpace& space);
	int          updatePosition(Vec3& center);
	Vec3&		 getPosition();

	int			 registerEvent(EVENT_ID ev, Vec3& location, float distance);
	int			 removeEvent(EVENT_ID ev);

	void		 registerApproachingCallback(IApproachingCallback* approachingCallback);
	void		 registerEventCallback();
	void		 registerActiveSpaceUpdate();


	WorldNavigator() = default;
	virtual ~WorldNavigator() = default;
};