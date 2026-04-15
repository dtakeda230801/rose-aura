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
	mObjectRepository	= std::make_unique<ObjectActivator>();
	mWorldNavigator		= std::make_unique<WorldNavigator>();
	mSoundCoordinator   = std::make_unique<SoundCoordinator>();
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

IObjectActivator& RoseAuraImpl::getObjectRepository()
{
	return *mObjectRepository;
}

IWorldNavigator& RoseAuraImpl::getWorldNavigator()
{
	return *mWorldNavigator;
}

ISoundCoordinator& RoseAuraImpl::getSoundCoordinator()
{
	return *mSoundCoordinator;
}

