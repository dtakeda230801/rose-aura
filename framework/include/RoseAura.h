#pragma once

#include <memory>
#include "ICentralLooper.h"
#include "IGraphicsManager.h"
#include "IInputHandler.h"
#include "IObjectRepository.h"
#include "IWorldNavigator.h"

class RoseAura {
public:
	static std::unique_ptr<RoseAura> create();

	virtual ICentralLooper&		getCentralLooper()		= 0;
	virtual IGraphicsManager&	getGraphicsManager()	= 0;
	virtual IInputHandler&		getInputHandler()		= 0;
	virtual IObjectRepository&	getObjectRepository()	= 0;
	virtual IWorldNavigator&	getWorldNavigator()		= 0;

	virtual ~RoseAura() = default;
protected:
	RoseAura() = default;
};