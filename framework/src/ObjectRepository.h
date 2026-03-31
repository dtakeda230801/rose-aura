#pragma once

#include <vector>

#include "IObjectRepository.h"

class ObjectRepository : public IObjectRepository {
public:
	struct ObjectEntry {
		OBJECT_ID			 mId;
		void*				 mInstance;
		ObjectBinder	     mBinder;
		std::vector<TAG_ID>  mTags;
	};

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	OBJECT_ID registerObject(ObjectBinder binder);
	OBJECT_ID registerObject(ObjectBinder binder, std::vector<TAG_ID>& tags);
	RARetCode unregisterObject(OBJECT_ID id);
	RARetCode addTag(OBJECT_ID id, std::vector<TAG_ID>& tags);
	RARetCode removeTag(OBJECT_ID id, TAG_ID tag);

	RARetCode activate(OBJECT_ID id);
	RARetCode deactivate(OBJECT_ID id);

	RARetCode activateByTag(TAG_ID id);
	RARetCode deactivateByTag(TAG_ID id);

	bool isActivate(OBJECT_ID id);
	bool isActivateByTag(TAG_ID id);

	ObjectRepository();
	virtual ~ObjectRepository();

private:
	ObjectEntry* searchObjectEntry(OBJECT_ID id);
	std::vector<ObjectRepository::ObjectEntry*>
		searchObjectEntryByTag(TAG_ID id);

	RARetCode activateInternal(ObjectEntry* entry);
	RARetCode deactivateInternal(ObjectEntry* entry);

	unsigned int	mIdCounter;

	std::vector<std::unique_ptr<ObjectEntry>> mObjects;
};