#include "WorldNavigator.h"

void WorldNavigator::createInstance()
{
	if (!mInstance) {
		mInstance = std::unique_ptr<WorldNavigator>(new WorldNavigator());
	}
}

void WorldNavigator::destroyInstance()
{
	if (mInstance) {
		mInstance.reset();
	}
}

WorldNavigator& WorldNavigator::getInstance()
{
	return *mInstance;
}

unsigned int WorldNavigator::createWorld(SpaceConfig& space)
{
	return 0;
}

unsigned int WorldNavigator::getCurrentWorld()
{
	return 0;
}

int WorldNavigator::changeWorld(unsigned int id)
{
	return 0;
}



std::unique_ptr<WorldNavigator> WorldNavigator::mInstance = nullptr;