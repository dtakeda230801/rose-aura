#include "RoseAura.h"
#include "RoseAuraImpl.h"

std::unique_ptr<RoseAura> RoseAura::create()
{
	return std::make_unique<RoseAuraImpl>();
}

RoseAuraImpl::RoseAuraImpl()
{
	mCentralLooper		= std::make_unique<CentralLooper>();
	mGraphicsManager	= std::make_unique<GraphicsManager>();
	mInputHandler		= std::make_unique<InputHandler>();
	mObjectRepository	= std::make_unique<ObjectRepository>();
	mWorldNavigator		= std::make_unique<WorldNavigator>();
	mMediaCoordinator   = std::make_unique<MediaCoordinator>();
}

ICentralLooper& RoseAuraImpl::getCentralLooper()
{
	return *mCentralLooper;
}

IGraphicsManager& RoseAuraImpl::getGraphicsManager()
{
	return *mGraphicsManager;
}

IInputHandler& RoseAuraImpl::getInputHandler()
{
	return *mInputHandler;
}

IObjectRepository& RoseAuraImpl::getObjectRepository()
{
	return *mObjectRepository;
}

IWorldNavigator& RoseAuraImpl::getWorldNavigator()
{
	return *mWorldNavigator;
}

IMediaCoordinator& RoseAuraImpl::getMediaCoordinator()
{
	return *mMediaCoordinator;
}

