#include "WorldNavigator.h"

WorldNavigator::WORLD_ID	 WorldNavigator::createWorld(SpaceConfig& space)
{
	return 0;
}

WorldNavigator::WORLD_ID	 WorldNavigator::getCurrentWorld()
{
	return 0;
}

int WorldNavigator::setCurrentWorld(WORLD_ID id)
{
	return 0;
}

WorldNavigator::WORLD_ID	 WorldNavigator::changeWorld(WORLD_ID id)
{
	return 0;
}

WorldNavigator::ActiveSpace& WorldNavigator::getActiveSpace()
{
	ActiveSpace a;
	return a;
}

int WorldNavigator::setActiveSpace(ActiveSpace& space)
{
	return 0;
}

int WorldNavigator::updatePosition(Vec3& center)
{
	return 0;
}

WorldNavigator::Vec3& WorldNavigator::getPosition()
{
	Vec3 a;
	return a;
}

int	WorldNavigator::registerEvent(EVENT_ID ev, Vec3& location, float distance)
{
	return 0;
}

int WorldNavigator::removeEvent(EVENT_ID ev)
{
	return 0;
}

void WorldNavigator::registerApproachingCallback(IApproachingCallback* approachingCallback)
{

}
void WorldNavigator::registerEventCallback()
{
}

void WorldNavigator::registerActiveSpaceUpdate()
{
}

