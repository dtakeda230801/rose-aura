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

void	ObjectRepository::registerObject(Object* obj)
{
}

void    ObjectRepository::removeObject(Object* obj)
{
}

ObjectRepository::Object* ObjectRepository::getObject(unsigned int id)
{
	return nullptr;
}



std::unique_ptr<ObjectRepository> ObjectRepository::mInstance = nullptr;