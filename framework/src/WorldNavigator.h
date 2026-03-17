#pragma once

#include "IWorldNavigator.h"

class WorldNavigator : public IWorldNavigator 
{
public:
	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	unsigned int createWorld(SpaceConfig& space);
	unsigned int getCurrentWorld();
	int			 changeWorld(unsigned int id);

	WorldNavigator() = default;
	virtual ~WorldNavigator() = default;
};