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

std::unique_ptr<WorldNavigator> WorldNavigator::mInstance = nullptr;