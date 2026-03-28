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

	world.mId               = mWorlds.size() + 1;
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

	if (mWorlds.size() == 1) {
		mCurrentWorld = &(mWorlds[0]);
	}

	return world.mId;
}

WorldNavigator::WORLD_ID WorldNavigator::getCurrentWorld()
{
	return mCurrentWorld->mId;
}

RARetCode WorldNavigator::changeWorld(WORLD_ID id)
{
	std::lock_guard<std::mutex> lock(mMutex);
	for (auto world : mWorlds) {
		if (world.mId == id) {
			mCurrentWorld = &world;
			return RARetCode::RET_OK;
		}
	}

	return RARetCode::RET_ERR_INVALID_ARG;
}

WorldNavigator::Bounds WorldNavigator::getActiveSpace()
{
	return mCurrentWorld->mActiveSpace;
}

WorldNavigator::Vec3 WorldNavigator::getActiveSpacePosition()
{
	return calcCenter(mCurrentWorld->mActiveSpace);
}

RARetCode WorldNavigator::moveActiveSpace(Vec3& pos)
{
	std::lock_guard<std::mutex> lock(mMutex);
	mCurrentWorld->mActiveSpace = calcBounds(pos, mCurrentWorld->mActiveRange);
	return RARetCode::RET_OK;
}

void WorldNavigator::resetActiveSpace()
{
	std::lock_guard<std::mutex> lock(mMutex);
	mCurrentWorld->mActiveSpace = calcBounds(mCurrentWorld->mPosition, mCurrentWorld->mActiveRange);;
}

RARetCode WorldNavigator::movePosition(Vec3& pos)
{
	std::lock_guard<std::mutex> lock(mMutex);
	Vec3 previous = mCurrentWorld->mPosition;

	if (!fitWithin(mCurrentWorld->mWorldSpace, pos)) {
		mCurrentWorld->mPosition = adjustPosition(mCurrentWorld->mWorldSpace, mCurrentWorld->mPosition, pos);
		return RARetCode::RET_ADJUSTED;
	}

	mCurrentWorld->mPosition = pos;

	if (mCurrentWorld->mLimitScrolling)
	{
		if (!fitWithin(mCurrentWorld->mNonScrollSpace, pos)) {
			Vec3& scrollPos = mCurrentWorld->mScrollPosition;
			Vec3& current   = mCurrentWorld->mPosition;

			scrollPos.mX = scrollPos.mX + (current.mX - previous.mX);
			scrollPos.mY = scrollPos.mY + (current.mY - previous.mY);
			scrollPos.mX = scrollPos.mZ + (current.mZ - previous.mZ);

			mCurrentWorld->mNonScrollSpace = calcBounds(scrollPos, mCurrentWorld->mNonScrollRange);

			if (mCurrentWorld->mEnableFollowing) {
				mCurrentWorld->mActiveSpace = calcBounds(scrollPos, mCurrentWorld->mActiveRange);
			}
		}
		else if (mCurrentWorld->mEnableFollowing) {
			mCurrentWorld->mActiveSpace = calcBounds(mCurrentWorld->mPosition, mCurrentWorld->mActiveRange);
		}
	}
	return RARetCode::RET_OK;
}

WorldNavigator::Vec3 WorldNavigator::getPosition()
{
	return mCurrentWorld->mPosition;
}

RARetCode WorldNavigator::registerTrigger(TRIGGER_ID id, Vec3& location, float distance)
{
	if (!fitWithin(mCurrentWorld->mWorldSpace, location)) {
		return RARetCode::RET_ERR_INVALID_PARAMS;
	}

	std::lock_guard<std::mutex> lock(mMutex);
	mCurrentWorld->mTriggers.emplace_back(id, location, distance);
	return RARetCode::RET_OK;
}

RARetCode WorldNavigator::removeTrigger(TRIGGER_ID id)
{
	RARetCode ret = RARetCode::RET_OK;

	std::lock_guard<std::mutex> lock(mMutex);
	std::vector<Trigger>& triggers = mCurrentWorld->mTriggers;

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

	mTriggerCallbacks.erase(
		std::remove(mTriggerCallbacks.begin(), mTriggerCallbacks.end(), cb), mTriggerCallbacks.end());
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
	: mCurrentWorld(nullptr)
{
}


//////////////////////////////////////////////////////////
// Private
//////////////////////////////////////////////////////////

float WorldNavigator::calcDistance(int a, int b)
{
	float fa = a;
	float fb = b;

	return static_cast<float>(std::sqrt((a * a) + (b * b)));
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
	ret.mY = checkCrossing(base.mMin.mZ, base.mMax.mZ, current.mZ, next.mZ);

	return ret;
}

int WorldNavigator::checkCrossing(int max, int min, int current, int next)
{
	int		ret;
	float	tmin, tmax;

	tmin = static_cast<float>(max - current) / static_cast<float>(next - current);
	tmax = static_cast<float>(min - current) / static_cast<float>(next - current);

	if (tmin < 1.0f) {
		ret = min;
	}
	else if (tmax < 1.0f) {
		ret = max;
	}
	else {
		ret= next;
	}
	return ret;
}
