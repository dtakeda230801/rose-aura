#include "RoseAura.h"
#include "RoseAuraImpl.h"

#include "res.h"

RoseAuraResources* RoseAuraResources::mIns = nullptr;

std::unique_ptr<RoseAura> RoseAura::create()
{
	return std::make_unique<RoseAuraImpl>();
}

RoseAuraImpl::RoseAuraImpl()
{
	mCentralLooper = std::make_unique<CentralLooper>();
	mGraphicsManager = std::make_unique<GraphicsManager>();
	mInputHandler = std::make_unique<InputHandler>();
	mObjectRepository = std::make_unique<ObjectActivator>();
	mWorldNavigator = std::make_unique<WorldNavigator>();
	mSoundCoordinator = std::make_unique<SoundCoordinator>();
	mConditionSaver = std::make_unique<ConditionSaver>();
	mStoryAnchor = std::make_unique<StoryAnchor>();

	RoseAuraResources::create();
}

RoseAuraImpl::~RoseAuraImpl()
{
	RoseAuraResources::destroy();
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

IObjectActivator& RoseAuraImpl::getObjectActivator()
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

IConditionSaver& RoseAuraImpl::getConditionSaver()
{
	return *mConditionSaver;
}

IStoryAnchor& RoseAuraImpl::getStoryAnchor()
{
	return *mStoryAnchor;
}
