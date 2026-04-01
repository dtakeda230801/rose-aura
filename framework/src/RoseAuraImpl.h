#pragma once

#include "RoseAura.h"
#include "CentralLooper.h"
#include "GraphicsManager.h"
#include "InputHandler.h"
#include "ObjectRepository.h"
#include "WorldNavigator.h"
#include "MediaCoordinator.h"

class RoseAuraImpl : public RoseAura{
public:
	ICentralLooper&		getCentralLooper();
	IGraphicsManager& 	getGraphicsManager();
	IInputHandler& 		getInputHandler();
	IObjectRepository& 	getObjectRepository();
	IWorldNavigator&	getWorldNavigator();
	IMediaCoordinator&  getMediaCoordinator();

	RoseAuraImpl();
	virtual ~RoseAuraImpl() = default;

private:
	std::unique_ptr<CentralLooper>		mCentralLooper;
	std::unique_ptr<GraphicsManager>	mGraphicsManager;
	std::unique_ptr<InputHandler>		mInputHandler;
	std::unique_ptr<ObjectRepository>	mObjectRepository;
	std::unique_ptr<WorldNavigator>		mWorldNavigator;
	std::unique_ptr<MediaCoordinator>	mMediaCoordinator;
};