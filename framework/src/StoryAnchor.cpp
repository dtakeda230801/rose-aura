
#include <nlohmann/json.hpp>
#include <fstream>

#include "StoryAnchor.h"
#include "Utility.h"

using json = nlohmann::json;


/////////////////////////////////////
/////////////////////////////////////
RARetCode StoryAnchor::loadStoryGraph(const char* graph)
{
	std::ifstream ifs(graph);
	if (!ifs) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}

	json j = json::parse(ifs);

	mStoryPoints.clear();

	for (const auto& sp : j["story_point"]) {
		StoryPoint newSp = {};

		newSp.mName		     = sp["name"];
		newSp.mStatus        = StoryPointState::HIDDEN;
		newSp.mAutoAvailable = sp["auto_available"];
		newSp.mAutoDisabled  = sp["auto_disabled"];

		for (const auto& aReq : sp["available_req"]) {
			std::string reqSp         = aReq["story_point"];
			StoryPointState reqStatus = convertState(aReq["state"]);
			newSp.mAvailableRequirement
				.emplace_back(std::pair<std::string, StoryPointState>(reqSp, reqStatus));
		}

		for (const auto& aReq : sp["disabled_req"]) {
			std::string reqSp = aReq["story_point"];
			StoryPointState reqStatus = convertState(aReq["state"]);
			newSp.mDisabledRequirement
				.emplace_back(std::pair<std::string, StoryPointState>(reqSp, reqStatus));
		}
		mStoryPoints.push_back(newSp);
	}
	return RARetCode::RET_OK;
}

RARetCode StoryAnchor::registerStoryPointCallback(IStoryPointCallback* cb)
{
	if (!cb) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}

	std::lock_guard<std::mutex> lock(mMutex);
	mCallbacks.push_back(cb);
	return RARetCode::RET_OK;
}

RARetCode StoryAnchor::unregisterStoryPointCallback(IStoryPointCallback* cb)
{
	if (!cb) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}

	std::lock_guard<std::mutex> lock(mMutex);
	if (0 != Utility::eraseVectorElm(mCallbacks, cb)) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}

	return RARetCode::RET_OK;
}

RARetCode StoryAnchor::changeState(std::string storyPoint, StoryPointState state)
{
	for (auto& sp : mStoryPoints) {
		if (storyPoint == sp.mName) {
			if (tryToChangeState(sp, state)) {
				sp.mStatus = state;

				std::lock_guard<std::mutex> lock(mMutex);
				for (auto& cb : mCallbacks) {
					cb->onStateChanged(storyPoint, state);
				}

				performAllStoryPointUpdate();

				return RARetCode::RET_OK;
			}
		}
	}
	return RARetCode::RET_ERR_INVALID_STATE;
}

RARetCode StoryAnchor::getStoryPointsState(std::string storyPoint, StoryPointState& state)
{
	for (auto& sp : mStoryPoints) {
		if (sp.mName == storyPoint) {
			state = sp.mStatus;
			return RARetCode::RET_OK;
		}
	}

	return RARetCode::RET_ERR_INVALID_ARG;
}


std::vector<std::string>
StoryAnchor::getStoryPointsByState(StoryPointState state)
{
	std::vector<std::string> ret;

	for (auto& sp : mStoryPoints) {
		if (sp.mStatus == state) {
			ret.push_back(sp.mName);
		}
	}
	return ret;
}

StoryAnchor::StoryAnchor() {

}

StoryAnchor::~StoryAnchor() {

}

/////////////////////////////////////
/////////////////////////////////////
bool StoryAnchor::checkCondition(std::vector<std::pair<std::string, StoryPointState>> conds)
{
	bool ret = true;

	for (auto& sp : mStoryPoints) {
		for (auto& cond : conds) {
			if (cond.first == sp.mName) {
				if (cond.second != sp.mStatus) {
					ret = false;
					break;
				}
			}
		}
		if (!ret) {
			break;
		}
	}

	return ret;
}

bool StoryAnchor::tryToChangeState(StoryPoint& storyPoint, StoryPointState next)
{
	bool ret = false;

	switch (next) {
	case StoryPointState::HIDDEN:
		break;
	case StoryPointState::AVAILABLE:
		if (checkCondition(storyPoint.mAvailableRequirement)) {
			ret = true;
		}
		break;
	case StoryPointState::COMPLETED:
		ret = true;
		break;
	case StoryPointState::DISABLED:
		if (checkCondition(storyPoint.mDisabledRequirement)) {
			ret = true;
		}
		break;
	default:
		break;
	};

	return ret;
}

void StoryAnchor::performAllStoryPointUpdate()
{
	for (auto& sp : mStoryPoints) {
		if (sp.mAutoAvailable && sp.mStatus == StoryPointState::HIDDEN) {
			if (tryToChangeState(sp, StoryPointState::AVAILABLE)) {
				sp.mStatus = StoryPointState::AVAILABLE;
				for (auto& cb : mCallbacks) {
					cb->onStateChanged(sp.mName, StoryPointState::AVAILABLE);
				}
			}
		}

		if (sp.mAutoDisabled && sp.mStatus != StoryPointState::COMPLETED) {
			if (tryToChangeState(sp, StoryPointState::DISABLED)) {
				sp.mStatus = StoryPointState::DISABLED;
				for (auto& cb : mCallbacks) {
					cb->onStateChanged(sp.mName, StoryPointState::DISABLED);
				}
			}
		}
	}
}

IStoryAnchor::StoryPointState StoryAnchor::convertState(std::string state)
{
	if (state == "available") {
		return StoryPointState::AVAILABLE;
	}

	if (state == "completed") {
		return StoryPointState::COMPLETED;
	}

	if (state == "disabled") {
		return StoryPointState::DISABLED;
	}

	return StoryPointState::HIDDEN;
}
