#pragma once

#include "IObjectRepository.h"

class ObjectRepository : public IObjectRepository {
public:
	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	OBJECT_ID registerObject(std::unique_ptr<ObjectBinder> binder);
	RARetCode unregisterObject(OBJECT_ID id);
	RARetCode addTag(OBJECT_ID id, std::vector<TAG_ID>& tags);
	RARetCode removeTag(OBJECT_ID id, TAG_ID tag);

	RARetCode activate(OBJECT_ID id);
	RARetCode deactivate(OBJECT_ID id);

	RARetCode activateByTag(TAG_ID id);
	RARetCode deactivateByTag(TAG_ID id);

	ObjectRepository() = default;
	virtual ~ObjectRepository() = default;
};