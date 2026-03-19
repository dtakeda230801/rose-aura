#include <memory>
#include <mutex>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "RoseAura.h"

#include "Utility.h"

#include "raylib.h"

class ContinuousInputTask : 
	  public ICentralLooper::ITask
	, public ICentralLooper::IFrameSyncCallback {
public:
	//ITask
	int doTask()
	{
		mInputHandler.update();
		return 0;
	};
	int finish()
	{
		return 0;
	};
	std::string getTaskId()
	{
		return "Test Task";
	}

	//IFrameSyncCallback
	int sync()
	{
		mCentralLooper.setTask(this);
		return 0;
	}

	ContinuousInputTask(ICentralLooper& centralLooper, IInputHandler& inputHandler) :
		 mCentralLooper(centralLooper)
		,mInputHandler(inputHandler)
	{
	}

	~ContinuousInputTask() = default;
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
	};

	~BGRenderer() = default;
	BGRenderer() = default;
};

class TxTRenderer : public IGraphicsManager::IObjectRenderer
{
public:
	void render()
	{
		DrawText("Hello, raylib!", 190, 200, 30, DARKGRAY);
	};

	~TxTRenderer() = default;
	TxTRenderer() = default;
};

class DotRenderer :
	  public IGraphicsManager::IObjectRenderer
	, public IInputHandler::IInputHandlerCallback
{
public:

	void render()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		DrawCircle(mX, mY, 10, SKYBLUE);
	};

	void onEvent(std::vector<std::pair<InputState, InputType>>& events)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		for (auto event : events) {
			//Utility::printLog("MyDot Input(%d / %d)", event.first, event.second);
			InputState state = event.first;
			InputType  type  = event.second;

			if (state == PUSHED || state == PRESSED) {
				if (type == UP) {
					mY -= 5;
				}
				else if (type == DOWN) {
					mY += 5;
				}
				else if (type == LEFT) {
					mX -= 5;
				}
				else if (type == RIGHT) {
					mX += 5;
				}
			}

			if (state == PUSHED && type == ACTION1) {
				if (mTextOn) {
					mTextOn = false;
					mGraphicsManager.removeRenderer(mTxtRenderer);
				}
				else {
					mTextOn = true;
					mGraphicsManager.setRenderer(mTxtRenderer);
				}
			}
		}
	}


	DotRenderer(IGraphicsManager& graphicsManager, IObjectRenderer* txtRenderer):
		mGraphicsManager(graphicsManager),mTxtRenderer(txtRenderer)
	{
	};

	~DotRenderer() = default;

private:
	IGraphicsManager& mGraphicsManager;

	std::mutex		   mMutex;

	unsigned int mX = 420;
	unsigned int mY = 215;

	bool         mTextOn = true;

	IObjectRenderer*	mTxtRenderer;
};

std::string readInputConf()
{
	std::ifstream file("dummy_game\\input_map.json");

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
		std::unique_ptr<RoseAura> rose_aura = RoseAura::create();

		ICentralLooper&		centralLooper	= rose_aura->getCentralLooper();
		IInputHandler&		inputHandler	= rose_aura->getInputHandler();
		IGraphicsManager&	graphicsManager = rose_aura->getGraphicsManager();

		ContinuousInputTask* inputTask = new ContinuousInputTask(centralLooper,inputHandler);

		BGRenderer*		bgRenderer	   = new BGRenderer();
		TxTRenderer*	txtRenderer	   = new TxTRenderer();
		DotRenderer*	dotRenderer    = new DotRenderer(graphicsManager, txtRenderer);

		////////////////////////////////////////////
		centralLooper.registerFrameSyncCallback(inputTask);
		centralLooper.setTask(inputTask);

		inputHandler.setConf(readInputConf());
		inputHandler.registerCallback(dotRenderer);

		graphicsManager.setRenderer(bgRenderer);
		graphicsManager.setRenderer(txtRenderer);
		graphicsManager.setRenderer(dotRenderer);

		////////////////////////////////////////////
		centralLooper.start(60);

		graphicsManager.runUntilClosed();

		centralLooper.stop();

		////////////////////////////////////////////


		delete dotRenderer;
		delete txtRenderer;
		delete bgRenderer;

		delete inputTask;
	}
	_CrtDumpMemoryLeaks();

	return 0;
}