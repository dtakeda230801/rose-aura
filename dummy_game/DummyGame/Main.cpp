#include <memory>
#include <mutex>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>

#include "RoseAura.h"
#include "RoseAuraReturnCode.h"

#include "Utility.h"
#include "raylib.h"

using namespace RoseAuraReturnCode;

#define WIN_SIZE_W 800
#define WIN_SIZE_H 600

#define TXT_POS_X 10
#define TXT_POS_Y 10

#define CIRCLE_SIZE 10

IWorldNavigator::WorldConfig	gWorldConf;
IObjectRepository::TAG_ID		gDummyGameTag = 0x01;

void buildConf1()
{
	gWorldConf.mWorldSpace.mMin.mX = 100;
	gWorldConf.mWorldSpace.mMin.mY = 100;
	gWorldConf.mWorldSpace.mMin.mZ = 0;
	gWorldConf.mWorldSpace.mMax.mX = 700;
	gWorldConf.mWorldSpace.mMax.mY = 500;
	gWorldConf.mWorldSpace.mMax.mZ = 0;

	gWorldConf.mActiveRange.mX = 100;
	gWorldConf.mActiveRange.mY = 100;
	gWorldConf.mActiveRange.mZ = 100;

	gWorldConf.mNonScrollRange.mX = 50;
	gWorldConf.mNonScrollRange.mY = 50;
	gWorldConf.mNonScrollRange.mZ = 50;

	gWorldConf.mPosition.mX = 400;
	gWorldConf.mPosition.mY = 300;
	gWorldConf.mPosition.mZ = 0;
	
	gWorldConf.mEnableFollowing = true;
	gWorldConf.mLimitScrolling  = true;
}

////////////////////////////////////////////
////////////////////////////////////////////
class ContinuousInputTask :
	  public ICentralLooper::ITask
	, public ICentralLooper::IFrameSyncCallback {
public:
	//ITask
	void doTask()
	{
		mInputHandler.update();
	};

	void finish()
	{
	};

	std::string getTaskName()
	{
		return "Test Task";
	}

	//IFrameSyncCallback
	void sync()
	{
		mCentralLooper.enqueueTask(this);
	}

	void init()
	{
		mCentralLooper.registerFrameSyncCallback(this);
		mCentralLooper.enqueueTask(this);
	}

	void fin()
	{
		mCentralLooper.unregisterFrameSyncCallback(this);
	}


	ContinuousInputTask(ICentralLooper& centralLooper, IInputHandler& inputHandler) :
		 mCentralLooper(centralLooper)
		,mInputHandler(inputHandler)
	{
	}

	virtual ~ContinuousInputTask() = default;
private:
	ICentralLooper& mCentralLooper;
	IInputHandler&	mInputHandler;
};

////////////////////////////////////////////
////////////////////////////////////////////
class BGRenderer : public IGraphicsManager::IObjectRenderer
{
public:
	void render()
	{
		ClearBackground(RAYWHITE);

		Rectangle rect = { static_cast<float>(gWorldConf.mWorldSpace.mMin.mX)
			             , static_cast<float>(gWorldConf.mWorldSpace.mMin.mY)
			             , static_cast<float>(gWorldConf.mWorldSpace.mMax.mX - gWorldConf.mWorldSpace.mMin.mX)
			             , static_cast<float>(gWorldConf.mWorldSpace.mMax.mY - gWorldConf.mWorldSpace.mMin.mY) };
		DrawRectangleLinesEx(rect, 3.0f, LIGHTGRAY);
	};

	void init()
	{
		mGraphicsManager.setRenderer(this);
	}

	void fin()
	{
		mGraphicsManager.removeRenderer(this);
	}

	BGRenderer(IGraphicsManager& objR) :
		mGraphicsManager(objR)
	{
	}

	virtual ~BGRenderer() = default;

private:
	IGraphicsManager& mGraphicsManager;
};

////////////////////////////////////////////
////////////////////////////////////////////
class ActiveSpaceRenderer : public IGraphicsManager::IObjectRenderer
	                      , public IWorldNavigator::IActiveSpaceCallback
{
public:
	void render()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		Rectangle rect = { static_cast<float>(mX)
						  ,static_cast<float>(mY)
			              ,static_cast<float>(mW)
			              ,static_cast<float>(mH) };
		DrawRectangleLinesEx(rect, 3.0f, BLUE);
	};

	void onUpdate(IWorldNavigator::WORLD_ID worldId, IWorldNavigator::Bounds activeSpace)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mX = activeSpace.mMin.mX;
		mY = activeSpace.mMin.mY;
		mW = activeSpace.mMax.mX - activeSpace.mMin.mX;
		mH = activeSpace.mMax.mY - activeSpace.mMin.mY; 
	}

	void init()
	{
		mWorldNavigator.registerActiveSpaceCallback(this);
		mGraphicsManager.setRenderer(this);
	}

	void fin()
	{
		mGraphicsManager.removeRenderer(this);
		mWorldNavigator.unregisterActiveSpaceCallback();
	}

	ActiveSpaceRenderer(IGraphicsManager& gm, IWorldNavigator& wn) : 
		mGraphicsManager(gm), mWorldNavigator(wn)
	{

	}

	virtual ~ActiveSpaceRenderer() = default;

private:
	IGraphicsManager& mGraphicsManager;
	IWorldNavigator&  mWorldNavigator;

	std::mutex mMutex;
	unsigned int mX = gWorldConf.mPosition.mX - gWorldConf.mActiveRange.mX;
	unsigned int mY = gWorldConf.mPosition.mY - gWorldConf.mActiveRange.mY;
	unsigned int mW = gWorldConf.mActiveRange.mX*2;
	unsigned int mH = gWorldConf.mActiveRange.mY*2;
};

////////////////////////////////////////////
////////////////////////////////////////////
class TxTRenderer : public IGraphicsManager::IObjectRenderer
			  	  , public IInputHandler::IInputHandlerCallback
{
public:
	//IObjectRenderer
	void render()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		if (mDisplay) {
			DrawText("Rose Aura Dummy Game", TXT_POS_X, TXT_POS_Y, 30, DARKGRAY);
		}
	};

	//IInputHandlerCallback
	void onEvent(std::vector<std::pair<InputState, InputType>>& events)
	{
		for (auto event : events) {
			InputState state = event.first;
			InputType  type = event.second;

			if (state == InputState::PUSHED && type == InputType::ACTION1) {
				if (mDisplay) {
					mDisplay = false;
				}
				else {
					mDisplay = true;
				}
			}
		}
	}

	void init()
	{
		mInputHandler.registerCallback(this);
		mGraphicsManager.setRenderer(this);
	}

	void fin()
	{
		mGraphicsManager.removeRenderer(this);
		mInputHandler.unregisterCallback(this);
	}

	TxTRenderer(IGraphicsManager& gm, IInputHandler& ih) :
		 mGraphicsManager(gm)
		,mInputHandler(ih)
	{
	}
	virtual ~TxTRenderer() = default;

private:
	IGraphicsManager&	mGraphicsManager;
	IInputHandler&		mInputHandler;

	std::mutex		mMutex;
	bool			mDisplay = true;

};

////////////////////////////////////////////
////////////////////////////////////////////
class DotRenderer :
	  public IGraphicsManager::IObjectRenderer
	, public IInputHandler::IInputHandlerCallback
{
public:

	void render()
	{
		IWorldNavigator::Vec3 pos = mWorldNavigator.getPosition();
		DrawCircle(pos.mX, pos.mY, CIRCLE_SIZE, SKYBLUE);
	};

	void onEvent(std::vector<std::pair<InputState, InputType>>& events)
	{
		for (auto event : events) {
			//Utility::printLog("MyDot Input(%d / %d)", event.first, event.second);
			InputState state = event.first;
			InputType  type  = event.second;

			IWorldNavigator::Vec3 pos = mWorldNavigator.getPosition();

			if (state == InputState::PUSHED || state == InputState::PRESSED) {
				if (type == InputType::UP) {
					pos.mY -= 5;
				}
				else if (type == InputType::DOWN) {
					pos.mY += 5;
				}
				else if (type == InputType::LEFT) {
					pos.mX -= 5;
				}
				else if (type == InputType::RIGHT) {
					pos.mX += 5;
				}
				mWorldNavigator.movePosition(pos);
			}
		}
	}

	void init()
	{
		mGraphicsManager.setRenderer(this);
		mInputHandler.registerCallback(this);
	}

	void fin()
	{
		mInputHandler.unregisterCallback(this);
		mGraphicsManager.removeRenderer(this);
	}

	DotRenderer(IWorldNavigator&  worldNavigator, IGraphicsManager& gm, IInputHandler& ih):
		  mWorldNavigator(worldNavigator)
		, mGraphicsManager(gm)
		, mInputHandler(ih)
	{
	};

	virtual ~DotRenderer() = default;

private:
	IWorldNavigator&  mWorldNavigator;
	IGraphicsManager& mGraphicsManager;
	IInputHandler&    mInputHandler;

	bool       mTextOn = true;
};

////////////////////////////////////////////
////////////////////////////////////////////
class DotTrigger :public IGraphicsManager::IObjectRenderer
			    , public IWorldNavigator::ITriggerCallback
{
public:
	//IObjectRenderer
	void render()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		DrawCircle(mPosition.mX, mPosition.mY, CIRCLE_SIZE, mColor);
	};

	//ITriggerCallback
	bool onApproaching(IWorldNavigator::WORLD_ID	worldId
		             , IWorldNavigator::TRIGGER_ID	eventId
					 , IWorldNavigator::Vec3&		trigerLocation
		             , IWorldNavigator::Vec3&		position)
	{
		//Utility::printLog("Approaching...");

		bool ret = false;
		if (CIRCLE_SIZE > calcDistance(trigerLocation, position)) {
			ret = true;
		}
		else {
			std::lock_guard<std::mutex> lock(mMutex);
			mColor = RED;
		}
		return ret;
	};

	void onTrigger(IWorldNavigator::WORLD_ID	worldId
		         , IWorldNavigator::TRIGGER_ID	eventId)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mColor = PINK;
	};

	void init()
	{
		mGraphicsManager.setRenderer(this);
		mWorldNavigator.registerTrigger(mId, mPosition, mDistance, this);
	}

	void fin()
	{
		mWorldNavigator.removeTrigger(mId);
		mGraphicsManager.removeRenderer(this);
	}

	DotTrigger(IGraphicsManager& gm, IWorldNavigator& wn) :
		  mGraphicsManager(gm)
		, mWorldNavigator(wn)
	{
	};
	virtual ~DotTrigger() = default;

private:
	float calcDistance(IWorldNavigator::Vec3& a, IWorldNavigator::Vec3& b)
	{
		return static_cast<float>(std::sqrt(
			  (static_cast<double>(b.mX - a.mX) * static_cast<double>(b.mX - a.mX))
			+ (static_cast<double>(b.mY - a.mY) * static_cast<double>(b.mY - a.mY))
			+ (static_cast<double>(b.mZ - a.mZ) * static_cast<double>(b.mZ - a.mZ))));
	}
	
	IGraphicsManager& mGraphicsManager;
	IWorldNavigator&  mWorldNavigator;

	std::mutex					mMutex;
	IWorldNavigator::TRIGGER_ID mId		  = 1;
	IWorldNavigator::Vec3		mPosition = { 200,200, 0 };
	float						mDistance = 30.f;
	Color						mColor    = RED;

};
////////////////////////////////////////////
////////////////////////////////////////////
class SoundTester : public IInputHandler::IInputHandlerCallback
{
public:
	//IInputHandlerCallback
	void onEvent(std::vector<std::pair<InputState, InputType>>& events)
	{
		for (auto event : events) {
			InputState state = event.first;
			InputType  type = event.second;

			if (state == InputState::PUSHED && type == InputType::ACTION2) {
				mSoundCoordinator.test();
			}
		}
	}

	void init()
	{
		mInputHandler.registerCallback(this);
	}

	void fin()
	{
		mInputHandler.unregisterCallback(this);
	}

	void loadWaveFile() {

	}

	SoundTester(IInputHandler& ih, ISoundCoordinator& mc) :
		mInputHandler(ih)
		, mSoundCoordinator(mc)
	{
	}
	virtual ~SoundTester() = default;

private:
	IInputHandler& mInputHandler;
	ISoundCoordinator& mSoundCoordinator;
};

////////////////////////////////////////////
////////////////////////////////////////////
std::string readInputConf()
{
	std::ifstream file("input_map.json");

	if (!file) {
		Utility::printLog("can not read input_map.json");
		return "";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();

	return buffer.str();
}

////////////////////////////////////////////
////////////////////////////////////////////
int main()
{
	{ //for _CrtDumpMemoryLeaks()
		////////////////////////////////////////////

		buildConf1();
		IObjectRepository::OBJECT_ID id;
		std::vector<IObjectRepository::OBJECT_ID>	ids;
		std::vector<IObjectRepository::TAG_ID>		tags = { gDummyGameTag };

		////////////////////////////////////////////
		std::unique_ptr<RoseAura> rose_aura = RoseAura::create();

		ICentralLooper&	   centralLooper	= rose_aura->getCentralLooper();
		IInputHandler&	   inputHandler	    = rose_aura->getInputHandler();
		IGraphicsManager&  graphicsManager  = rose_aura->getGraphicsManager();
		IWorldNavigator&   worldNavigator   = rose_aura->getWorldNavigator();
		IObjectRepository& objectRepository = rose_aura->getObjectRepository();
		ISoundCoordinator& mediaCoordinator = rose_aura->getSoundCoordinator();

		////////////////////////////////////////////
		id = objectRepository.registerObject(
			objectRepository.makeObjectBinder<ContinuousInputTask, ICentralLooper&, IInputHandler&>(
				  &ContinuousInputTask::init , &ContinuousInputTask::fin
				, centralLooper , inputHandler)
			, tags
		);
		ids.push_back(id);

		id = objectRepository.registerObject(
			objectRepository.makeObjectBinder<DotTrigger, IGraphicsManager&, IWorldNavigator&>(
				  &DotTrigger::init	, &DotTrigger::fin
				, graphicsManager , worldNavigator)
			, tags
		);
		ids.push_back(id);

		id = objectRepository.registerObject(
			objectRepository.makeObjectBinder<BGRenderer, IGraphicsManager&>(
				  &BGRenderer::init	, &BGRenderer::fin
				, graphicsManager )
			, tags
		);
		ids.push_back(id);

		id = objectRepository.registerObject(
			objectRepository.makeObjectBinder<ActiveSpaceRenderer, IGraphicsManager&, IWorldNavigator&>(
				  &ActiveSpaceRenderer::init , &ActiveSpaceRenderer::fin
				, graphicsManager , worldNavigator )
			, tags
		);
		ids.push_back(id);

		id = objectRepository.registerObject(
			objectRepository.makeObjectBinder<TxTRenderer, IGraphicsManager&, IInputHandler&>(
				  &TxTRenderer::init , &TxTRenderer::fin
				, graphicsManager , inputHandler)
			, tags
		);
		ids.push_back(id);

		id = objectRepository.registerObject(
			objectRepository.makeObjectBinder<DotRenderer, IWorldNavigator&, IGraphicsManager&, IInputHandler&>(
				  &DotRenderer::init , &DotRenderer::fin
				, worldNavigator , graphicsManager , inputHandler)
			, tags
		);
		ids.push_back(id);

		id = objectRepository.registerObject(
			objectRepository.makeObjectBinder<SoundTester, IInputHandler&, ISoundCoordinator&>(
				&SoundTester::init, &SoundTester::fin
				, inputHandler, mediaCoordinator)
			, tags
		);
		ids.push_back(id);

		////////////////////////////////////////////
		IWorldNavigator::WORLD_ID wId = worldNavigator.createWorld(gWorldConf);
		inputHandler.setConf(readInputConf());

		objectRepository.activateByTag(gDummyGameTag);

		////////////////////////////////////////////
		mediaCoordinator.start();
		centralLooper.start(30);
		graphicsManager.runUntilClosed();

		////////////////////////////////////////////
		centralLooper.stop();
		mediaCoordinator.stop();
		objectRepository.deactivateByTag(gDummyGameTag);

	}
	_CrtDumpMemoryLeaks();

	return 0;
}