#include "ObjectRepository.h"

void ObjectRepository::createInstance()
{
	if (!mInstance) {
		mInstance = std::unique_ptr<ObjectRepository>(new ObjectRepository());
	}
}

void ObjectRepository::destroyInstance()
{
	if (mInstance) {
		mInstance.reset();
	}
}

ObjectRepository& ObjectRepository::getInstance()
{
	return *mInstance;
}

std::unique_ptr<ObjectRepository> ObjectRepository::mInstance = nullptr;