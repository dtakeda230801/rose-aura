#include <cmath>

#include "WorldNavigator.h"
#include "Utility.h"

//////////////////////////////////////////////////////////
// APIs
//////////////////////////////////////////////////////////
WorldNavigator::WORLD_ID WorldNavigator::createWorld(WorldConfig& conf)
{
	std::lock_guard<std::mutex> lock(mMutex);
	World world;

	world.mId               = static_cast<unsigned int>(mWorlds.size()) + 1;
	world.mWorldSpace       = conf.mWorldSpace;
	world.mActiveRange      = conf.mActiveRange;
	world.mActiveSpace      = calcBounds(conf.mPosition, conf.mActiveRange);
	world.mNonScrollRange   = conf.mNonScrollRange;
	world.mNonScrollSpace   = calcBounds(conf.mPosition, conf.mNonScrollRange);
	world.mEnableFollowing  = conf.mEnableFollowing;
	world.mLimitScrolling   = conf.mLimitScrolling;
	world.mPosition         = conf.mPosition;
	world.mScrollPosition   = conf.mPosition;

	mWorlds.push_back(world);

	if (mCurrentWorldIndex == -1) {
		mCurrentWorldIndex = 0;
	}

	return world.mId;
}

WorldNavigator::WORLD_ID WorldNavigator::getCurrentWorld()
{
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
	return mWorlds[mCurrentWorldIndex].mActiveSpace;
}

WorldNavigator::Vec3 WorldNavigator::getActiveSpacePosition()
{
	return calcCenter(mWorlds[mCurrentWorldIndex].mActiveSpace);
}

RARetCode WorldNavigator::moveActiveSpace(Vec3& pos)
{
	std::lock_guard<std::mutex> lock(mMutex);
	mWorlds[mCurrentWorldIndex].mActiveSpace = calcBounds(pos, mWorlds[mCurrentWorldIndex].mActiveRange);
	doActiveSpaceCallbacks();
	return RARetCode::RET_OK;
}

void WorldNavigator::resetActiveSpace()
{
	std::lock_guard<std::mutex> lock(mMutex);
	mWorlds[mCurrentWorldIndex].mActiveSpace 
		= calcBounds(mWorlds[mCurrentWorldIndex].mPosition, mWorlds[mCurrentWorldIndex].mActiveRange);
	doActiveSpaceCallbacks();
}

RARetCode WorldNavigator::movePosition(Vec3& pos)
{
	RARetCode ret		 = RARetCode::RET_OK;
	bool	  doCallback = false;

	std::lock_guard<std::mutex> lock(mMutex);
	Vec3 previous = mWorlds[mCurrentWorldIndex].mPosition;

	if (!fitWithin(mWorlds[mCurrentWorldIndex].mWorldSpace, pos)) {
		mWorlds[mCurrentWorldIndex].mPosition 
			= adjustPosition(mWorlds[mCurrentWorldIndex].mWorldSpace, mWorlds[mCurrentWorldIndex].mPosition, pos);
		ret = RARetCode::RET_ADJUSTED;
	}
	else {
		mWorlds[mCurrentWorldIndex].mPosition = pos;
	}

	if (mWorlds[mCurrentWorldIndex].mLimitScrolling)
	{
		if (!fitWithin(mWorlds[mCurrentWorldIndex].mNonScrollSpace, pos)) {
			Vec3& scrollPos = mWorlds[mCurrentWorldIndex].mScrollPosition;
			Vec3& current   = mWorlds[mCurrentWorldIndex].mPosition;

			scrollPos.mX = scrollPos.mX + (current.mX - previous.mX);
			scrollPos.mY = scrollPos.mY + (current.mY - previous.mY);
			scrollPos.mX = scrollPos.mZ + (current.mZ - previous.mZ);

			mWorlds[mCurrentWorldIndex].mNonScrollSpace 
				= calcBounds(scrollPos, mWorlds[mCurrentWorldIndex].mNonScrollRange);

			if (mWorlds[mCurrentWorldIndex].mEnableFollowing) {
				mWorlds[mCurrentWorldIndex].mActiveSpace = calcBounds(scrollPos, mWorlds[mCurrentWorldIndex].mActiveRange);
				doCallback = true;
			}
		}
	} else if (mWorlds[mCurrentWorldIndex].mEnableFollowing) {
		mWorlds[mCurrentWorldIndex].mActiveSpace 
			= calcBounds(mWorlds[mCurrentWorldIndex].mPosition, mWorlds[mCurrentWorldIndex].mActiveRange);
		doCallback = true;
	}

	if (doCallback) {
		doActiveSpaceCallbacks();
	}

	for (Trigger t : mWorlds[mCurrentWorldIndex].mTriggers) {
		if (t.mDistance >= calcDistance(t.mLocation, mWorlds[mCurrentWorldIndex].mPosition)) {
			for (ITriggerCallback* cb : mTriggerCallbacks) {
				if (cb->onApproaching(mWorlds[mCurrentWorldIndex].mId, t.mId, t.mLocation)) {
					cb->onTrigger(mWorlds[mCurrentWorldIndex].mId, t.mId);
				}
			}
		}
	}

	return ret;
}

WorldNavigator::Vec3 WorldNavigator::getPosition()
{
	return mWorlds[mCurrentWorldIndex].mPosition;
}

RARetCode WorldNavigator::registerTrigger(TRIGGER_ID id, Vec3& location, float distance)
{
	if (!fitWithin(mWorlds[mCurrentWorldIndex].mWorldSpace, location)) {
		return RARetCode::RET_ERR_INVALID_PARAMS;
	}

	std::lock_guard<std::mutex> lock(mMutex);

	for (auto trigger : mWorlds[mCurrentWorldIndex].mTriggers)
	{
		if (trigger.mId == id) {
			return RARetCode::RET_ERR_INVALID_PARAMS;
		}
	}

	mWorlds[mCurrentWorldIndex].mTriggers.emplace_back(id, location, distance);
	return RARetCode::RET_OK;
}

RARetCode WorldNavigator::removeTrigger(TRIGGER_ID id)
{
	RARetCode ret = RARetCode::RET_OK;

	std::lock_guard<std::mutex> lock(mMutex);
	std::vector<Trigger>& triggers = mWorlds[mCurrentWorldIndex].mTriggers;

	auto newEnd = std::remove_if(triggers.begin(), triggers.end(),
		[id](const Trigger& trigger) {
			return trigger.mId == id;
		});
	if (newEnd != triggers.end()) {
		triggers.erase(newEnd, triggers.end());
	}
	else {
		ret = RARetCode::RET_ERR_INVALID_ARG;
	}

	return ret;
}

RARetCode WorldNavigator::registerTriggerCallback(ITriggerCallback* cb)
{
	std::lock_guard<std::mutex> lock(mMutex);
	if (!cb) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}
	mTriggerCallbacks.push_back(cb);
	return RARetCode::RET_OK;
}

RARetCode WorldNavigator::unregisterTriggerCallback(ITriggerCallback* cb)
{
	std::lock_guard<std::mutex> lock(mMutex);
	if (!cb) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}

	if (0 != Utility::eraseVectorElm(mTriggerCallbacks, cb)) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}
	return RARetCode::RET_OK;
}

RARetCode WorldNavigator::registerActiveSpaceUpdate(IActiveSpaceCallback* cb)
{
	std::lock_guard<std::mutex> lock(mMutex);
	if (!cb) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}
	mActiveSpaceCallbacks.push_back(cb);
	return RARetCode::RET_OK;
}

RARetCode WorldNavigator::unregisterActiveSpaceUpdate(IActiveSpaceCallback* cb)
{
	std::lock_guard<std::mutex> lock(mMutex);
	if (!cb) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}

	if (0 != Utility::eraseVectorElm(mActiveSpaceCallbacks, cb)) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}
	return RARetCode::RET_OK;
}


WorldNavigator::WorldNavigator()
	: mCurrentWorldIndex(-1)
{
}


//////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////

float WorldNavigator::calcDistance(Vec3& a, Vec3& b)
{
	return static_cast<float>(std::sqrt( (static_cast<double>(b.mX - a.mX) * static_cast<double>(b.mX - a.mX))
		                               + (static_cast<double>(b.mY - a.mY) * static_cast<double>(b.mY - a.mY))
							           + (static_cast<double>(b.mZ - a.mZ) * static_cast<double>(b.mZ - a.mZ)) ));
}

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
  	 && base.mMax.mY < target.mY
	 && base.mMax.mZ < target.mZ) {
		return false;
	}

	if (base.mMin.mX > target.mX
 	 && base.mMin.mY > target.mY
	 && base.mMin.mZ > target.mZ) {
		return false;
	}

	return true;
}

WorldNavigator::Vec3 WorldNavigator::adjustPosition(Bounds& base
	                                              , Vec3& current
	                                              , Vec3& next)
{
	Vec3 ret;
	ret.mX = checkCrossing(base.mMin.mX, base.mMax.mX, current.mX, next.mX);
	ret.mY = checkCrossing(base.mMin.mY, base.mMax.mY, current.mY, next.mY);
	ret.mZ = checkCrossing(base.mMin.mZ, base.mMax.mZ, current.mZ, next.mZ);

	return ret;
}

int WorldNavigator::checkCrossing(int min, int max, int current, int next)
{
	int		ret;
	float	tmin, tmax;

	tmin = static_cast<float>(min - current) / static_cast<float>(next - current);
	tmax = static_cast<float>(max - current) / static_cast<float>(next - current);

	if (0.0f < tmin && tmin < 1.0f) {
		ret = min;
	}
	else if (0.0f < tmax && tmax < 1.0f) {
		ret = max;
	}
	else {
		ret= next;
	}
	return ret;
}

void WorldNavigator::doActiveSpaceCallbacks()
{
	for (IActiveSpaceCallback* cb : mActiveSpaceCallbacks) {
		cb->onUpdate(mWorlds[mCurrentWorldIndex].mId, mWorlds[mCurrentWorldIndex].mActiveSpace);
	}
}
