#include "ObjectActivator.h"
#include "Utility.h"

ObjectActivator::OBJECT_ID 
	ObjectActivator::registerObject(std::unique_ptr<ObjectBinder> binder)
{
	std::vector<TAG_ID> tags = {};
	return registerObject(std::move(binder), tags);
}

ObjectActivator::OBJECT_ID 
	ObjectActivator::registerObject(std::unique_ptr<ObjectBinder> binder, std::vector<TAG_ID>& tags)
{
	OBJECT_ID id = ++mIdCounter;

	std::unique_ptr<ObjectEntry> entry = std::make_unique<ObjectEntry>();
	entry->mId       = id;
	entry->mInstance = nullptr;
	entry->mBinder   = std::move(binder);

	if (!tags.empty()) {
		entry->mTags.assign(tags.begin(), tags.end());
	}

	mObjects.push_back(std::move(entry));

	return id;
}


RARetCode ObjectActivator::unregisterObject(OBJECT_ID id)
{
	RARetCode ret = RARetCode::RET_OK;

	ObjectEntry* objEntry = searchObjectEntry(id);
	if (!objEntry) {
		return RARetCode::RET_ERR_NOT_FOUND;
	}

	deactivateInternal(objEntry);
	objEntry->mBinder->destroyParams(objEntry->mBinder->params);

	auto newEnd = std::remove_if(mObjects.begin(), mObjects.end(),
		[id](const std::unique_ptr<ObjectEntry>& entry) {
			return entry->mId == id;
		});
	if (newEnd != mObjects.end()) {
		mObjects.erase(newEnd, mObjects.end());
	} else {
		ret = RARetCode::RET_ERR_NOT_FOUND;
	}
	return ret;
}

RARetCode ObjectActivator::addTag(OBJECT_ID id, TAG_ID tag)
{
	ObjectEntry* objEntry = searchObjectEntry(id);
	if (!objEntry) {
		return RARetCode::RET_ERR_NOT_FOUND;
	}

	std::vector<TAG_ID>& objTags = objEntry->mTags;
	objTags.push_back(tag);

	return RARetCode::RET_OK;
}

RARetCode ObjectActivator::removeTag(OBJECT_ID id, TAG_ID tag)
{
	ObjectEntry* objEntry = searchObjectEntry(id);
	if (!objEntry) {
		return RARetCode::RET_ERR_NOT_FOUND;
	}

	std::vector<TAG_ID>& objTags = objEntry->mTags;

	if (0 != Utility::eraseVectorElm(objTags, tag)) {
		return RARetCode::RET_ERR_NOT_FOUND;
	}

	return RARetCode::RET_OK;
}

RARetCode ObjectActivator::activate(OBJECT_ID id)
{
	ObjectEntry* objEntry = searchObjectEntry(id);
	if (!objEntry) {
		return RARetCode::RET_ERR_NOT_FOUND;
	}

	return activateInternal(objEntry);
}

RARetCode ObjectActivator::deactivate(OBJECT_ID id)
{
	ObjectEntry* objEntry = searchObjectEntry(id);
	if (!objEntry) {
		return RARetCode::RET_ERR_NOT_FOUND;
	}

	return deactivateInternal(objEntry);
}

RARetCode ObjectActivator::activateByTag(TAG_ID id)
{
	std::vector<ObjectEntry*> entries = searchObjectEntryByTag(id);
	if (entries.empty()) {
		return RARetCode::RET_ERR_NOT_FOUND;
	}

	RARetCode ret = RARetCode::RET_OK;
	for (ObjectEntry* entry : entries) {
		if (activateInternal(entry) != RARetCode::RET_OK) {
			ret = RARetCode::RET_ERR_INVALID_STATE;
		}
	}

	return ret;
}
RARetCode ObjectActivator::deactivateByTag(TAG_ID id)
{
	std::vector<ObjectEntry*> entries = searchObjectEntryByTag(id);
	if (entries.empty()) {
		return RARetCode::RET_ERR_NOT_FOUND;
	}

	RARetCode ret = RARetCode::RET_OK;
	for (ObjectEntry* entry : entries) {
		if (deactivateInternal(entry) != RARetCode::RET_OK) {
			ret = RARetCode::RET_ERR_INVALID_STATE;
		}
	}

	return ret;
}

bool ObjectActivator::isActivate(OBJECT_ID id) 
{
	ObjectEntry* objEntry = searchObjectEntry(id);
	if (!objEntry) {
		return false;
	}

	return (objEntry->mInstance);
}

bool ObjectActivator::isActivateByTag(TAG_ID id)
{
	std::vector<ObjectEntry*> entries = searchObjectEntryByTag(id);
	if (entries.empty()) {
		return false;
	}

	bool ret = true;
	for (ObjectEntry* entry : entries) {
		if (!entry->mInstance) {
			ret = false;
		}
	}
	return ret;
}

ObjectActivator::ObjectActivator() :
	mIdCounter(0)
{
}

ObjectActivator::~ObjectActivator()
{
	for (std::unique_ptr<ObjectEntry>& entry : mObjects) {
		deactivateInternal(entry.get());
		entry->mBinder->destroyParams(entry->mBinder->params);
	}
}


ObjectActivator::ObjectEntry* 
	ObjectActivator::searchObjectEntry(OBJECT_ID id)
{
	for (auto& entry : mObjects)
	{
		if (entry->mId == id) {
			return entry.get();
		}
	}
	return nullptr;
}

std::vector<ObjectActivator::ObjectEntry*>
	ObjectActivator::searchObjectEntryByTag(TAG_ID id)
{
	std::vector<ObjectActivator::ObjectEntry*> entries = {};

	for (auto& entry : mObjects)
	{
		std::vector<TAG_ID>& tags = entry->mTags;

		if (std::find(tags.begin(), tags.end(), id) != tags.end())
		{
			entries.push_back(entry.get());
		}
	}
	return entries;
}

RARetCode ObjectActivator::activateInternal(ObjectEntry* entry)
{
	if (entry->mInstance) {
		return RARetCode::RET_ERR_INVALID_STATE;
	}
	ObjectBinder* binder = entry->mBinder.get();
	entry->mInstance = binder->create(binder->params);
	return RARetCode::RET_OK;
}

RARetCode ObjectActivator::deactivateInternal(ObjectEntry* entry)
{
	if (!entry->mInstance) {
		return RARetCode::RET_ERR_INVALID_STATE;
	}
	ObjectBinder* binder = entry->mBinder.get();
	binder->destroy(entry->mInstance);
	entry->mInstance = nullptr;
	return RARetCode::RET_OK;
}
