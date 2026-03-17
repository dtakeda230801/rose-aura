#include <memory>

#include "GraphicsManager.h"
#include "CentralLooper.h"
#include "InputHandler.h"
#include "Utility.h"

#include "raylib.h"

class LocalInputTask : public CentralLooper::ITask {
public:
	int doTask()
	{
		Utility::printLog("do Task");
		InputHandler::getInstance().update();
		return 0;
	};
	int finish()
	{
		Utility::printLog("finish");
		return 0;
	};
	std::string getTaskId()
	{
		return "Test Task";
	}
	
	LocalInputTask()  = default;
	~LocalInputTask() = default;
};

class FrameSync : public CentralLooper::IFrameSyncCallback {
public:
	FrameSync(CentralLooper::ITask* task) : mLocalTask(task), mCount(0) {
	}

	int sync()
	{
		mCount++;
		Utility::printLog("frame sync(%d)", mCount);
		CentralLooper::getInstance().setTask(mLocalTask);
		return 0;
	}

	~FrameSync() = default;

private:
	CentralLooper::ITask*	mLocalTask;
	int						mCount;
};

class InputEventCb : public InputHandler::IInputHandlerCallback {
public:
	void onEvent(Action action, unsigned short pushed)
	{
		Utility::printLog("input(%d / %d)", action, pushed);
	}
	InputEventCb()  = default;
	~InputEventCb() = default;
};

class BGRenderer : public GraphicsManager::IObjectRenderer
{
public:
	void render()
	{
		ClearBackground(RAYWHITE);
	};

	~BGRenderer() = default;
	BGRenderer() = default;
};

class TxTRenderer : public GraphicsManager::IObjectRenderer
{
public:
	void render()
	{
		DrawText("Hello, raylib!", 190, 200, 30, DARKGRAY);
	};

	~TxTRenderer() = default;
	TxTRenderer() = default;
};

class DotTRenderer : public GraphicsManager::IObjectRenderer
{
public:
	void render()
	{
		DrawCircle(420, 215, 10, SKYBLUE);
	};

	~DotTRenderer() = default;
	DotTRenderer()  = default;
};

class MyDot : public GraphicsManager::IObjectRenderer, public InputHandler::IInputHandlerCallback
{
public:

	void render()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		DrawCircle(mX, mY, 10, SKYBLUE);
	};

	void onEvent(Action action, unsigned short pushed)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		Utility::printLog("MyDot Input(%d / %d)", action, pushed);

		if (action == Down || action == Pressed) {
			if (pushed == 0x0001) {
				mY -= 5;
			}
			else if (pushed == 0x0002) {
				mY += 5;
			}
			else if (pushed == 0x0004) {
				mX -= 5;
			}
			else if (pushed == 0x0008) {
				mX += 5;
			}
		}
		if (action == Down && pushed == 0x1000) {
			if (mTextOn) {
				mTextOn = false;
				GraphicsManager::getInstance().removeRenderer(mTxtRenderer);
			}
			else {
				mTextOn = true;
				GraphicsManager::getInstance().setRenderer(mTxtRenderer);
			}
		}
	}


	MyDot(IObjectRenderer* txtRenderer):
		mTxtRenderer(txtRenderer)
	{
	};

	~MyDot() = default;

private:

	std::mutex		   mMutex;

	unsigned int mX = 420;
	unsigned int mY = 215;

	bool         mTextOn = true;

	IObjectRenderer*	mTxtRenderer;
};


int main() {
	Utility::printLog("TEST");

	////////////////////////////////////////////
	CentralLooper::createInstance();
	GraphicsManager::createInstance();
	InputHandler::createInstance();

	LocalInputTask*	 localInputTask = new LocalInputTask();
	FrameSync*		 fsync			= new FrameSync(localInputTask);
	InputEventCb*    inEvCb			= new InputEventCb();

	BGRenderer*		bgRenderer  = new BGRenderer();
	TxTRenderer*	txtRenderer = new TxTRenderer();
	DotTRenderer*	dotRenderer = new DotTRenderer();

	MyDot* myDot = new MyDot(txtRenderer);


	////////////////////////////////////////////
	CentralLooper::getInstance().setTask(localInputTask);
	CentralLooper::getInstance().registerFrameSyncCallback(fsync);
//	InputHandler::getInstance().registerCallback(inEvCb);
	InputHandler::getInstance().registerCallback(myDot);

	GraphicsManager::getInstance().setRenderer(bgRenderer);
	GraphicsManager::getInstance().setRenderer(txtRenderer);
//	GraphicsManager::getInstance().setRenderer(dotRenderer);
	GraphicsManager::getInstance().setRenderer(myDot);

	CentralLooper::getInstance().start(60);

	GraphicsManager::getInstance().runUntilClosed();

	CentralLooper::getInstance().stop();

	////////////////////////////////////////////
	InputHandler::destroyInstance();
	GraphicsManager::destroyInstance();
	CentralLooper::destroyInstance();

	delete myDot;

	delete dotRenderer;
	delete txtRenderer;
	delete bgRenderer;

	delete localInputTask;
	delete fsync;
	delete inEvCb;

	_CrtDumpMemoryLeaks();

	return 0;
}