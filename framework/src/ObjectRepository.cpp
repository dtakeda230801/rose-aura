#include "ObjectRepository.h"


ObjectRepository::OBJECT_ID 
	ObjectRepository::registerObject(std::unique_ptr<ObjectBinder> binder)
{
	return 0;
}

RARetCode ObjectRepository::unregisterObject(OBJECT_ID id)
{
	return RARetCode::RET_OK;
}

RARetCode ObjectRepository::addTag(OBJECT_ID id, std::vector<TAG_ID>& tags)
{
	return RARetCode::RET_OK;
}

RARetCode ObjectRepository::removeTag(OBJECT_ID id, TAG_ID tag)
{
	return RARetCode::RET_OK;
}

RARetCode ObjectRepository::activate(OBJECT_ID id)
{
	return RARetCode::RET_OK;
}

RARetCode ObjectRepository::deactivate(OBJECT_ID id)
{
	return RARetCode::RET_OK;
}

RARetCode ObjectRepository::activateByTag(TAG_ID id)
{
	return RARetCode::RET_OK;
}
RARetCode ObjectRepository::deactivateByTag(TAG_ID id)
{
	return RARetCode::RET_OK;
}
