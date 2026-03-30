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

IWorldNavigator::WorldConfig gWorldConf;

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

	BGRenderer() = default;
	virtual ~BGRenderer() = default;
};

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

	ActiveSpaceRenderer() = default;
	virtual ~ActiveSpaceRenderer() = default;

private:
	std::mutex mMutex;
	unsigned int mX = gWorldConf.mPosition.mX - gWorldConf.mActiveRange.mX;
	unsigned int mY = gWorldConf.mPosition.mY - gWorldConf.mActiveRange.mY;
	unsigned int mW = gWorldConf.mActiveRange.mX*2;
	unsigned int mH = gWorldConf.mActiveRange.mY*2;
};

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

	TxTRenderer() = default;
	virtual ~TxTRenderer() = default;

private:
	std::mutex		mMutex;
	bool			mDisplay = true;

};

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

	DotRenderer(IWorldNavigator&  worldNavigator):
		mWorldNavigator(worldNavigator)
	{
	};

	virtual ~DotRenderer() = default;

private:
	IWorldNavigator&  mWorldNavigator;

	bool       mTextOn = true;
};

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

	IWorldNavigator::TRIGGER_ID getId() {
		return mId;
	}

	IWorldNavigator::Vec3& getLocation() {
		return mPosition;
	}

	float getDistance() {
		return mDistance;
	}

	DotTrigger()		  = default;
	virtual ~DotTrigger() = default;

private:
	float calcDistance(IWorldNavigator::Vec3& a, IWorldNavigator::Vec3& b)
	{
		return static_cast<float>(std::sqrt(
			  (static_cast<double>(b.mX - a.mX) * static_cast<double>(b.mX - a.mX))
			+ (static_cast<double>(b.mY - a.mY) * static_cast<double>(b.mY - a.mY))
			+ (static_cast<double>(b.mZ - a.mZ) * static_cast<double>(b.mZ - a.mZ))));
	}
	
	std::mutex					mMutex;
	IWorldNavigator::TRIGGER_ID mId		  = 1;
	IWorldNavigator::Vec3		mPosition = { 200,200, 0 };
	float						mDistance = 30.f;
	Color						mColor    = RED;

};

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

int main()
{
	{ //for _CrtDumpMemoryLeaks()
		////////////////////////////////////////////
		buildConf1();

		////////////////////////////////////////////

		std::unique_ptr<RoseAura> rose_aura = RoseAura::create();

		ICentralLooper&		centralLooper	= rose_aura->getCentralLooper();
		IInputHandler&		inputHandler	= rose_aura->getInputHandler();
		IGraphicsManager&	graphicsManager = rose_aura->getGraphicsManager();
		IWorldNavigator&    worldNavigator  = rose_aura->getWorldNavigator();

		ContinuousInputTask* inputTask   = new ContinuousInputTask(centralLooper,inputHandler);

		DotTrigger*			 dotTrigger  = new DotTrigger();

		BGRenderer*		     bgRenderer	 = new BGRenderer();
		ActiveSpaceRenderer* asRenderer  = new ActiveSpaceRenderer();
		TxTRenderer*	     txtRenderer = new TxTRenderer();
		DotRenderer*	     dotRenderer = new DotRenderer(worldNavigator);

		////////////////////////////////////////////
		gWorldConf.mActiveSpaceCb = asRenderer;

		IWorldNavigator::WORLD_ID wId = worldNavigator.createWorld(gWorldConf);
		worldNavigator.registerTrigger(dotTrigger->getId(), dotTrigger->getLocation(), dotTrigger->getDistance(), dotTrigger);

		centralLooper.registerFrameSyncCallback(inputTask);
		centralLooper.enqueueTask(inputTask);

		inputHandler.setConf(readInputConf());
		inputHandler.registerCallback(txtRenderer);
		inputHandler.registerCallback(dotRenderer);

		graphicsManager.setRenderer(bgRenderer);
		graphicsManager.setRenderer(asRenderer);
		graphicsManager.setRenderer(txtRenderer);
		graphicsManager.setRenderer(dotTrigger);
		graphicsManager.setRenderer(dotRenderer);

		////////////////////////////////////////////
		centralLooper.start(30);

		graphicsManager.runUntilClosed();

		centralLooper.stop();

		////////////////////////////////////////////
		delete dotRenderer;
		delete txtRenderer;
		delete asRenderer;
		delete bgRenderer;

		delete dotTrigger;

		delete inputTask;
	}
	_CrtDumpMemoryLeaks();

	return 0;
}