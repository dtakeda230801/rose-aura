#include <cmath>

#include "WorldNavigator.h"
#include "MathematicalUtility.h"
#include "Utility.h"

//////////////////////////////////////////////////////////
// APIs
//////////////////////////////////////////////////////////
WorldNavigator::WORLD_ID WorldNavigator::createWorld(WorldConfig& conf)
{
	std::lock_guard<std::mutex> lock(mMutex);
	World world;

	world.mId						= static_cast<unsigned int>(mWorlds.size()) + 1;
	world.mWorldSpace				= conf.mWorldSpace;
	world.mActiveRange				= conf.mActiveRange;
	world.mActiveSpace				= calcBounds(conf.mPrimaryEntityPosition, conf.mActiveRange);
	world.mNonScrollRange			= conf.mNonScrollRange;
	world.mNonScrollSpace			= calcBounds(conf.mPrimaryEntityPosition, conf.mNonScrollRange);
	world.mEnableFollowing			= conf.mEnableFollowing;
	world.mLimitScrolling			= conf.mLimitScrolling;
	world.mPrimaryEntityPosition    = conf.mPrimaryEntityPosition;
	world.mScrollPosition			= conf.mPrimaryEntityPosition;
	world.mActiveSpaceCb			= nullptr;
	world.mCollisionCb				= nullptr;

	mWorlds.push_back(world);

	if (mCurrentWorldIndex == -1) {
		mCurrentWorldIndex = 0;
	}

	return world.mId;
}

WorldNavigator::WORLD_ID WorldNavigator::getCurrentWorld()
{
	std::lock_guard<std::mutex> lock(mMutex);
	return mWorlds[mCurrentWorldIndex].mId;
}

RARetCode WorldNavigator::changeWorld(WORLD_ID id)
{
	std::lock_guard<std::mutex> lock(mMutex);
	for (size_t i = 0; i < mWorlds.size(); i++) {
		if (mWorlds[i].mId == id) {
			mCurrentWorldIndex = static_cast<unsigned int >(i);
			return RARetCode::RET_OK;
		}

	}
	return RARetCode::RET_ERR_INVALID_ARG;
}

WorldNavigator::Bounds WorldNavigator::getActiveSpace()
{
	std::lock_guard<std::mutex> lock(mMutex);
	return mWorlds[mCurrentWorldIndex].mActiveSpace;
}

WorldNavigator::Vec3 WorldNavigator::getActiveSpacePosition()
{
	std::lock_guard<std::mutex> lock(mMutex);
	return calcCenter(mWorlds[mCurrentWorldIndex].mActiveSpace);
}

RARetCode WorldNavigator::moveActiveSpace(Vec3& pos)
{
	std::lock_guard<std::mutex> lock(mMutex);
	World& world = mWorlds[mCurrentWorldIndex];
	world.mActiveSpace = calcBounds(pos, world.mActiveRange);

	world.mActiveSpaceCb->onUpdate(world.mId, world.mActiveSpace);

	return RARetCode::RET_OK;
}

void WorldNavigator::resetActiveSpace()
{
	std::lock_guard<std::mutex> lock(mMutex);
	World& world = mWorlds[mCurrentWorldIndex];
	world.mActiveSpace = calcBounds(world.mPrimaryEntityPosition, world.mActiveRange);

	world.mActiveSpaceCb->onUpdate(world.mId, world.mActiveSpace);
}

RARetCode WorldNavigator::movePrimaryEntityPosition(Vec3& pos)
{
	RARetCode ret = RARetCode::RET_OK;

	bool doActiveAreaCallback = false;

	std::lock_guard<std::mutex> lock(mMutex);
	World& currentWorld = mWorlds[mCurrentWorldIndex];
	Vec3 previous       = currentWorld.mPrimaryEntityPosition;

	if (!fitWithin(currentWorld.mWorldSpace, pos)) {
		currentWorld.mPrimaryEntityPosition	= adjustPosition(currentWorld.mWorldSpace, pos);
		ret = RARetCode::RET_ADJUSTED;
	} else {
		currentWorld.mPrimaryEntityPosition = pos;
	}

	for (Entity& entity : currentWorld.mEntities) {
		if (entity.mDistance >= MathematicalUtility::calcDistance(entity.mLocation, pos)) {

			if (entity.mCb) {
				Vec3 to = currentWorld.mPrimaryEntityPosition;

				ICollisionCallback::CollisionResult colRret = entity.mCb->onApproaching(currentWorld.mId, PRIMARY_ENTITY_ID, previous, to);

				if (colRret == ICollisionCallback::CollisionResult::INHIBITED) {
					if (MathematicalUtility::calcDistance(previous, to) < MathematicalUtility::calcDistance(previous, currentWorld.mPrimaryEntityPosition)) {
						currentWorld.mPrimaryEntityPosition = to;
						ret = RARetCode::RET_ADJUSTED;
					}
				} else if (colRret == ICollisionCallback::CollisionResult::HIT){
					entity.mCb->onHit(currentWorld.mId, PRIMARY_ENTITY_ID);
				}
			}

			if (currentWorld.mCollisionCb) {
				Vec3 entityLocation = entity.mLocation;

				ICollisionCallback::CollisionResult colRret = currentWorld.mCollisionCb->onApproaching(currentWorld.mId, entity.mId, entityLocation, entityLocation);
				if (colRret == ICollisionCallback::CollisionResult::INHIBITED) {
					Utility::printLog("Ignore INHIBITED");
				} else if (colRret == ICollisionCallback::CollisionResult::HIT) {
					currentWorld.mCollisionCb->onHit(currentWorld.mId, PRIMARY_ENTITY_ID);
				}
			}
		}
	}

	if (currentWorld.mLimitScrolling)
	{
		if (!fitWithin(currentWorld.mNonScrollSpace, pos)) {
			Vec3& scrollPos = currentWorld.mScrollPosition;
			Vec3& current   = currentWorld.mPrimaryEntityPosition;

			scrollPos.mX = scrollPos.mX + (current.mX - previous.mX);
			scrollPos.mY = scrollPos.mY + (current.mY - previous.mY);
			scrollPos.mZ = scrollPos.mZ + (current.mZ - previous.mZ);

			currentWorld.mNonScrollSpace
				= calcBounds(scrollPos, currentWorld.mNonScrollRange);

			if (currentWorld.mEnableFollowing) {
				currentWorld.mActiveSpace = calcBounds(scrollPos, currentWorld.mActiveRange);
				doActiveAreaCallback = true;
			}
		}
	} else if (currentWorld.mEnableFollowing) {
		currentWorld.mActiveSpace
			= calcBounds(currentWorld.mPrimaryEntityPosition, currentWorld.mActiveRange);
		doActiveAreaCallback = true;
	}

	if (doActiveAreaCallback) {
		currentWorld.mActiveSpaceCb->onUpdate(currentWorld.mId, currentWorld.mActiveSpace);
	}

	return ret;
}

WorldNavigator::Vec3 WorldNavigator::getPrimaryEntityPosition()
{
	std::lock_guard<std::mutex> lock(mMutex);
	return mWorlds[mCurrentWorldIndex].mPrimaryEntityPosition;
}

RARetCode WorldNavigator::registerCollisionCallback(ICollisionCallback* cb)
{
	if (!cb) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}

	std::lock_guard<std::mutex> lock(mMutex);
	World& currentWorld = mWorlds[mCurrentWorldIndex];

	if (currentWorld.mCollisionCb) {
		return RARetCode::RET_ERR_INVALID_STATE;
	}

	currentWorld.mCollisionCb = cb;

	return RARetCode::RET_OK;
}

RARetCode WorldNavigator::unregisterCollisionCallback()
{
	std::lock_guard<std::mutex> lock(mMutex);
	World& currentWorld = mWorlds[mCurrentWorldIndex];

	if (!currentWorld.mActiveSpaceCb) {
		return RARetCode::RET_ERR_INVALID_STATE;
	}

	currentWorld.mCollisionCb = nullptr;

	return RARetCode::RET_OK;
}

RARetCode WorldNavigator::registerEntity(ENTITY_ID id, Vec3& location, float distance, ICollisionCallback* cb)
{
	if (!fitWithin(mWorlds[mCurrentWorldIndex].mWorldSpace, location)) {
		return RARetCode::RET_ERR_INVALID_PARAMS;
	}

	if (!cb) {
		return RARetCode::RET_ERR_INVALID_PARAMS;
	}

	std::lock_guard<std::mutex> lock(mMutex);

	for (auto& entity : mWorlds[mCurrentWorldIndex].mEntities)
	{
		if (entity.mId == id) {
			return RARetCode::RET_ERR_INVALID_PARAMS;
		}
	}

	mWorlds[mCurrentWorldIndex].mEntities.emplace_back(id, location, distance, cb);
	return RARetCode::RET_OK;
}

RARetCode WorldNavigator::removeEntity(ENTITY_ID id)
{
	std::lock_guard<std::mutex> lock(mMutex);

	RARetCode ret                  = RARetCode::RET_OK;
	std::vector<Entity>& entities = mWorlds[mCurrentWorldIndex].mEntities;

	auto newEnd = std::remove_if(entities.begin(), entities.end(),
		[id](const Entity& entity) {
			return entity.mId == id;
		});
	if (newEnd != entities.end()) {

		entities.erase(newEnd, entities.end());
	}
	else {
		ret = RARetCode::RET_ERR_INVALID_ARG;
	}

	return ret;
}

RARetCode	 WorldNavigator::moveEntity(ENTITY_ID id, Vec3& location) {
	std::lock_guard<std::mutex> lock(mMutex);

	RARetCode ret          = RARetCode::RET_OK;
	Entity*   targetEntity = findEntity(id);

	if (targetEntity == nullptr) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}

	Vec3 previous = targetEntity->mLocation;

	World& currentWorld = mWorlds[mCurrentWorldIndex];

	if (!fitWithin(currentWorld.mWorldSpace, location)) {
		targetEntity->mLocation = adjustPosition(currentWorld.mWorldSpace, location);
		ret = RARetCode::RET_ADJUSTED;
	} else {
		targetEntity->mLocation = location;
	}

	//////////////////////////
	if (targetEntity->mDistance >= MathematicalUtility::calcDistance(targetEntity->mLocation, currentWorld.mPrimaryEntityPosition)) {
		if (currentWorld.mCollisionCb) {
			Vec3 entityLocation = targetEntity->mLocation;

			ICollisionCallback::CollisionResult colRet
				= currentWorld.mCollisionCb->onApproaching(currentWorld.mId, targetEntity->mId, previous, entityLocation);
			if (colRet == ICollisionCallback::CollisionResult::INHIBITED) {
				targetEntity->mLocation = entityLocation;
				ret = RARetCode::RET_ADJUSTED;
			} else if (colRet == ICollisionCallback::CollisionResult::HIT) {
				currentWorld.mCollisionCb->onHit(currentWorld.mId, targetEntity->mId);
			}
		}

		if (targetEntity->mCb) {
			ICollisionCallback::CollisionResult colRet
				= targetEntity->mCb->onApproaching(currentWorld.mId, PRIMARY_ENTITY_ID, currentWorld.mPrimaryEntityPosition, currentWorld.mPrimaryEntityPosition);
			if (colRet == ICollisionCallback::CollisionResult::INHIBITED) {
				Utility::printLog("(%d) Ignore INHIBITED", targetEntity->mId);
			} else if (colRet == ICollisionCallback::CollisionResult::HIT) {
				targetEntity->mCb->onHit(currentWorld.mId, PRIMARY_ENTITY_ID);
			}
		}
	}

	//////////////////////////
	for (Entity& entity : currentWorld.mEntities) {
		if (entity.mId != targetEntity->mId &&
			entity.mDistance >= MathematicalUtility::calcDistance(targetEntity->mLocation, entity.mLocation)) {

			if (entity.mCb) {
				Vec3 to = targetEntity->mLocation;

				ICollisionCallback::CollisionResult colRet = entity.mCb->onApproaching(currentWorld.mId, targetEntity->mId, previous, to);

				if (colRet == ICollisionCallback::CollisionResult::INHIBITED) {
					if (MathematicalUtility::calcDistance(previous, to) < MathematicalUtility::calcDistance(previous, targetEntity->mLocation)) {
						targetEntity->mLocation = to;
						ret = RARetCode::RET_ADJUSTED;
					}
				}
				else if (colRet == ICollisionCallback::CollisionResult::HIT) {
					entity.mCb->onHit(currentWorld.mId, targetEntity->mId);
				}
			}
		}
	}

	return ret;
}

RARetCode WorldNavigator::getEntityLocation(ENTITY_ID	id, Vec3* location)
{
	std::lock_guard<std::mutex> lock(mMutex);

	if (location == nullptr) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}

	Entity* targetEntity = findEntity(id);

	if (targetEntity == nullptr) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}

	*location = targetEntity->mLocation;

	return RARetCode::RET_OK;
}

RARetCode WorldNavigator::registerActiveSpaceCallback(IActiveSpaceCallback* cb)
{
	if (!cb) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}

	std::lock_guard<std::mutex> lock(mMutex);
	World& currentWorld = mWorlds[mCurrentWorldIndex];

	if (currentWorld.mActiveSpaceCb) {
		return RARetCode::RET_ERR_INVALID_STATE;
	}

	currentWorld.mActiveSpaceCb = cb;

	return RARetCode::RET_OK;
}

RARetCode	 WorldNavigator::unregisterActiveSpaceCallback()
{
	std::lock_guard<std::mutex> lock(mMutex);
	World& currentWorld = mWorlds[mCurrentWorldIndex];

	if (!currentWorld.mActiveSpaceCb) {
		return RARetCode::RET_ERR_INVALID_STATE;
	}

	currentWorld.mActiveSpaceCb = nullptr;

	return RARetCode::RET_OK;
}


WorldNavigator::WorldNavigator()
	: mCurrentWorldIndex(-1)
{
}


//////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////

WorldNavigator::Vec3 WorldNavigator::calcCenter(Bounds& bounds)
{
	Vec3 ret;

	ret.mX = static_cast<int>(((float)bounds.mMin.mX + (float)bounds.mMax.mX) / 2.0f);
	ret.mY = static_cast<int>(((float)bounds.mMin.mY + (float)bounds.mMax.mY) / 2.0f);
	ret.mZ = static_cast<int>(((float)bounds.mMin.mZ + (float)bounds.mMax.mZ) / 2.0f);

	return ret;
}

WorldNavigator::Bounds WorldNavigator::calcBounds(Vec3& center, Extent& range)
{
	Bounds ret;

	ret.mMax.mX = center.mX + range.mX;
	ret.mMax.mY = center.mY + range.mY;
	ret.mMax.mZ = center.mZ + range.mZ;

	ret.mMin.mX = center.mX - range.mX;
	ret.mMin.mY = center.mY - range.mY;
	ret.mMin.mZ = center.mZ - range.mZ;

	return ret;
}


bool WorldNavigator::fitWithin(Bounds& base, Bounds& target)
{
	if (base.mMax.mX < target.mMax.mX
	 && base.mMax.mY < target.mMax.mY
	 && base.mMax.mZ < target.mMax.mZ) {
		return false;
	}

	if (base.mMin.mX > target.mMin.mX
 	 && base.mMin.mY > target.mMin.mY
	 && base.mMin.mZ > target.mMin.mZ) {
		return false;
	}

	return true;
}

bool WorldNavigator::fitWithin(Bounds& base, Vec3& target)
{
	if (base.mMax.mX < target.mX
  	 || base.mMax.mY < target.mY
	 || base.mMax.mZ < target.mZ) {
		return false;
	}

	if (base.mMin.mX > target.mX
 	 || base.mMin.mY > target.mY
	 || base.mMin.mZ > target.mZ) {
		return false;
	}

	return true;
}

WorldNavigator::Vec3 WorldNavigator::adjustPosition(Bounds& base
	                                              , Vec3&   pos)
{
	Vec3 ret;
	ret.mX = selectBoundaryPosition(base.mMin.mX, base.mMax.mX, pos.mX);
	ret.mY = selectBoundaryPosition(base.mMin.mY, base.mMax.mY, pos.mY);
	ret.mZ = selectBoundaryPosition(base.mMin.mZ, base.mMax.mZ, pos.mZ);

	return ret;
}

int WorldNavigator::selectBoundaryPosition(int min, int max, int val)
{
	if (val < min) {
		return min;
	}

	if (max < val) {
		return max;
	}

	return val;
}

WorldNavigator::Entity* WorldNavigator::findEntity(ENTITY_ID id)
{
	World& currentWorld = mWorlds[mCurrentWorldIndex];
	std::vector<Entity>& entities = currentWorld.mEntities;

	Entity* targetEntity = nullptr;

	for (auto& entity : entities) {
		if (entity.mId == id) {
			targetEntity = &entity;
		}
	}

	return targetEntity;
}
