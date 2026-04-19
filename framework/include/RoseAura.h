#pragma once

#include <memory>
#include "ICentralLooper.h"
#include "IGraphicsManager.h"
#include "IInputHandler.h"
#include "IObjectActivator.h"
#include "IWorldNavigator.h"
#include "ISoundCoordinator.h"
#include "IConditionSaver.h"
#include "IStoryAnchor.h"

class RoseAura {
public:
	static std::unique_ptr<RoseAura> create();

	virtual ICentralLooper&		getCentralLooper()		= 0;
	virtual IGraphicsManager&	getGraphicsManager()	= 0;
	virtual IInputHandler&		getInputHandler()		= 0;
	virtual IObjectActivator&	getObjectRepository()	= 0;
	virtual IWorldNavigator&	getWorldNavigator()		= 0;
	virtual ISoundCoordinator&  getSoundCoordinator()   = 0;
	virtual IConditionSaver&    getConditionSaver()     = 0;
	virtual IStoryAnchor&       getStoryAnchor()        = 0;


	virtual ~RoseAura() = default;
protected:
	RoseAura() = default;
};