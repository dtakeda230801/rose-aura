#pragma once

#include <mutex>

#include "IStoryAnchor.h"
#include "RoseAuraReturnCode.h"

using namespace RoseAuraReturnCode;

class StoryAnchor : public IStoryAnchor
{
public:
	 RARetCode loadStoryGraph(const char* graph);
	 RARetCode registerStoryPointCallback(IStoryPointCallback* cb);
	 RARetCode unregisterStoryPointCallback(IStoryPointCallback* cb);
	 RARetCode changeState(std::string storyPoint, StoryPointState state);
	 RARetCode getStoryPointsState(std::string storyPoint, StoryPointState& state);
	 std::vector<std::string>
		getStoryPointsByState(StoryPointState state);

	 StoryAnchor();
	 virtual ~StoryAnchor();

private:
	struct StoryPoint {
		std::string     mName;
		StoryPointState mStatus;
		std::vector<std::pair<std::string, StoryPointState>>
						mAvailableRequirement;
		std::vector<std::pair<std::string, StoryPointState>>
						mDisabledRequirement;
		bool            mAutoAvailable;
		bool            mAutoDisabled;
	};

	bool			checkCondition(std::vector<std::pair<std::string, StoryPointState>> conds);
	bool			tryToChangeState(StoryPoint& storyPoint, StoryPointState next);
	void			performAllStoryPointUpdate();
	StoryPointState convertState(std::string state);

	std::mutex  mMutex;
	std::vector<IStoryPointCallback*>
				mCallbacks;
	std::vector<StoryPoint>
				mStoryPoints;

};