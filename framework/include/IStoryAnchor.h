#pragma once

#include <string>
#include <vector>

#include "RoseAuraReturnCode.h"

class IStoryAnchor
{
public:
	enum class StoryPointState {
		  HIDDEN
		, AVAILABLE
		, COMPLETED
		, DISABLED
	};

	class IStoryPointCallback {
	public:

		virtual void onStateChanged(std::string storyPoint, StoryPointState state) = 0;

		IStoryPointCallback() = default;
		virtual ~IStoryPointCallback() = default;
	};

	virtual RARetCode loadStoryGraph(const char* graph)									  = 0;
	virtual RARetCode registerStoryPointCallback(IStoryPointCallback* cb)                 = 0;
	virtual RARetCode unregisterStoryPointCallback(IStoryPointCallback* cb)				  = 0;
	virtual RARetCode changeState(std::string storyPoint, StoryPointState state)          = 0;
	virtual RARetCode getStoryPointsState(std::string storyPoint, StoryPointState& state) = 0;
	virtual std::vector<std::string>
					  getStoryPointsByState(StoryPointState state)                        = 0;
};
