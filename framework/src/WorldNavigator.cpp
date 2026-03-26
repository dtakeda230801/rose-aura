#include "WorldNavigator.h"

WorldNavigator::WORLD_ID	 WorldNavigator::createWorld(WorldConfig& conf)
{
	unsigned int worldId = mWorlds.size() + 1;
	Vec3 position = { 0,0,0 };

	mWorlds.emplace_back(worldId, conf, position);

	if (mWorlds.size() == 1) {
		mCurrentWorld = &(mWorlds[0]);
	}

	return worldId;
}

WorldNavigator::WORLD_ID	 WorldNavigator::getCurrentWorld()
{
	return mCurrentWorld->mId;
}

WorldNavigator::WORLD_ID	 WorldNavigator::changeWorld(WORLD_ID id)
{
	for (auto world : mWorlds) {
		if (world.mId == id) {
			mCurrentWorld = &world;
			return 0;
		}
	}

	return -1;
}

WorldNavigator::ActiveSpace& WorldNavigator::getActiveSpace()
{
	return mCurrentWorld->mWorldCongig.mActiveSpace;
}

int WorldNavigator::setActiveSpace(ActiveSpace& space)
{
	mCurrentWorld->mWorldCongig.mActiveSpace = space;
	return 0;
}

int WorldNavigator::updatePosition(Vec3& pos)
{
	mCurrentWorld->mPosition = pos;
	return 0;
}

WorldNavigator::Vec3& WorldNavigator::getPosition()
{
	return 	mCurrentWorld->mPosition;
}

int	WorldNavigator::registerTrigger(TRIGGER_ID id, Vec3& location, float distance)
{
	mTriggers.emplace_back(id, location, distance);

	return 0;
}

int WorldNavigator::removeTrigger(TRIGGER_ID id)
{
	mTriggers.erase(
		std::remove_if(mTriggers.begin(), mTriggers.end(),
			[id](const Trigger& trigger) {
				return trigger.mId == id;
			}),
		mTriggers.end());


	return 0;
}

void WorldNavigator::registerApproachingCallback(IApproachingCallback* approachingCallback)
{
	mApproachingCallbacks.push_back(approachingCallback);
}

void WorldNavigator::unregisterApproachingCallback(IApproachingCallback* approachingCallback)
{
	mApproachingCallbacks.erase(
		std::remove(mApproachingCallbacks.begin(), mApproachingCallbacks.end(), approachingCallback), mApproachingCallbacks.end());
}

void WorldNavigator::registerTriggerCallback(ITriggerCallback* cb)
{
	mTriggerCallbacks.push_back(cb);
}

void WorldNavigator::unregisterTriggerCallback(ITriggerCallback* cb)
{
	mTriggerCallbacks.erase(
		std::remove(mTriggerCallbacks.begin(), mTriggerCallbacks.end(), cb), mTriggerCallbacks.end());
}

void WorldNavigator::registerActiveSpaceUpdate(IActiveSpaceCallback* cb)
{
	mActiveSpaceCallbacks.push_back(cb);
}

void WorldNavigator::unregisterActiveSpaceUpdate(IActiveSpaceCallback* cb)
{
	mActiveSpaceCallbacks.erase(
		std::remove(mActiveSpaceCallbacks.begin(), mActiveSpaceCallbacks.end(), cb), mActiveSpaceCallbacks.end());
}


WorldNavigator::WorldNavigator()
	: mCurrentWorld(nullptr)
{

}
