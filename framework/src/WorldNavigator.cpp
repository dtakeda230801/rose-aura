#include "WorldNavigator.h"

WorldNavigator::WORLD_ID	 WorldNavigator::createWorld(WorldConfig& conf)
{
	unsigned int worldId = mWorlds.size() + 1;
	Vec3 position = { 0,0,0 };

	mWorlds.emplace_back(worldId, conf, position);

	if (mWorlds.size() == 1) {
		mCurrentWorld = mWorlds[0];
	}

	return worldId;
}

WorldNavigator::WORLD_ID	 WorldNavigator::getCurrentWorld()
{
	return mCurrentWorld.mId;
}

WorldNavigator::WORLD_ID	 WorldNavigator::changeWorld(WORLD_ID id)
{
	for (auto world : mWorlds) {
		if (world.mId == id) {
			mCurrentWorld = world;
			return 0;
		}
	}

	return -1;
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

int	WorldNavigator::registerEvent(TRIGGER_ID trigger, Vec3& location, float distance)
{
	return 0;
}

int WorldNavigator::removeEvent(TRIGGER_ID trigger)
{
	return 0;
}

void WorldNavigator::registerApproachingCallback(IApproachingCallback* approachingCallback)
{

}

void WorldNavigator::unregisterApproachingCallback(IApproachingCallback* approachingCallback)
{

}

void WorldNavigator::registerTriggerCallback(ITriggerCallback* triggerCallback)
{

}

void WorldNavigator::unregisterTriggerCallback(ITriggerCallback* triggerCallback)
{

}

void WorldNavigator::registerActiveSpaceUpdate(IActiveSpaceCallback* activeSpaceCallback)
{

}

void WorldNavigator::unregisterActiveSpaceUpdate(IActiveSpaceCallback* activeSpaceCallback)
{

}
