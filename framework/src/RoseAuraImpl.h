#pragma once

#include "RoseAura.h"
#include "CentralLooper.h"
#include "GraphicsManager.h"
#include "InputHandler.h"
#include "ObjectActivator.h"
#include "WorldNavigator.h"
#include "SoundCoordinator.h"
#include "ConditionSaver.h"

class RoseAuraImpl : public RoseAura{
public:
	ICentralLooper&		getCentralLooper();
	IGraphicsManager& 	getGraphicsManager();
	IInputHandler& 		getInputHandler();
	IObjectActivator& 	getObjectRepository();
	IWorldNavigator&	getWorldNavigator();
	ISoundCoordinator&  getSoundCoordinator();
	IConditionSaver&    getConditionSaver();

	RoseAuraImpl();
	virtual ~RoseAuraImpl() = default;

private:
	std::unique_ptr<CentralLooper>		mCentralLooper;
	std::unique_ptr<GraphicsManager>	mGraphicsManager;
	std::unique_ptr<InputHandler>		mInputHandler;
	std::unique_ptr<ObjectActivator>	mObjectRepository;
	std::unique_ptr<WorldNavigator>		mWorldNavigator;
	std::unique_ptr<SoundCoordinator>	mSoundCoordinator;
	std::unique_ptr<ConditionSaver>	    mConditionSaver;

};