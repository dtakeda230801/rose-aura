#include <memory>
#include <mutex>

#include "RoseAura.h"

#include "Utility.h"

#include "raylib.h"

class LocalInputTask : public ICentralLooper::ITask {
public:
	int doTask()
	{
		//Utility::printLog("do Task");
		mInputHandler.update();
		return 0;
	};
	int finish()
	{
		//Utility::printLog("finish");
		return 0;
	};
	std::string getTaskId()
	{
		return "Test Task";
	}
	
	LocalInputTask(IInputHandler& inputHandler) :
		mInputHandler(inputHandler)
	{
	}

	~LocalInputTask() = default;
private:
	IInputHandler& mInputHandler;
};

class FrameSync : public ICentralLooper::IFrameSyncCallback {
public:
	FrameSync(ICentralLooper& centralLooper,ICentralLooper::ITask* task) : 
		mCentralLooper(centralLooper),mLocalTask(task), mCount(0) {
	}

	int sync()
	{
		mCount++;
		//Utility::printLog("frame sync(%d)", mCount);
		mCentralLooper.setTask(mLocalTask);
		return 0;
	}

	~FrameSync() = default;

private:
	ICentralLooper&			mCentralLooper;
	ICentralLooper::ITask*	mLocalTask;
	int						mCount;
};

class InputEventCb : public IInputHandler::IInputHandlerCallback {
public:
	void onEvent(std::vector<std::pair<InputState, InputType>>& events)
	{
		//Utility::printLog("input(%d / %d)", ev, pushed);
	}
	InputEventCb()  = default;
	~InputEventCb() = default;
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

class DotTRenderer : public IGraphicsManager::IObjectRenderer
{
public:
	void render()
	{
		DrawCircle(420, 215, 10, SKYBLUE);
	};

	~DotTRenderer() = default;
	DotTRenderer()  = default;
};

class MyDot : public IGraphicsManager::IObjectRenderer, public IInputHandler::IInputHandlerCallback
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


	MyDot(IGraphicsManager& graphicsManager, IObjectRenderer* txtRenderer):
		mGraphicsManager(graphicsManager),mTxtRenderer(txtRenderer)
	{
	};

	~MyDot() = default;

private:
	IGraphicsManager& mGraphicsManager;

	std::mutex		   mMutex;

	unsigned int mX = 420;
	unsigned int mY = 215;

	bool         mTextOn = true;

	IObjectRenderer*	mTxtRenderer;
};


int main() {
	{
		Utility::printLog("TEST");

		////////////////////////////////////////////
		std::unique_ptr<RoseAura> rose_aura = RoseAura::create();

		ICentralLooper&		centralLooper	= rose_aura->getCentralLooper();
		IInputHandler&		inputHandler	= rose_aura->getInputHandler();
		IGraphicsManager&	graphicsManager = rose_aura->getGraphicsManager();

		LocalInputTask* localInputTask = new LocalInputTask(inputHandler);
		FrameSync*		fsync		   = new FrameSync(centralLooper, localInputTask);
		InputEventCb*	inEvCb		   = new InputEventCb();

		BGRenderer*		bgRenderer	   = new BGRenderer();
		TxTRenderer*	txtRenderer	   = new TxTRenderer();
		DotTRenderer*	dotRenderer    = new DotTRenderer();

		MyDot* myDot = new MyDot(graphicsManager, txtRenderer);

		////////////////////////////////////////////
		centralLooper.setTask(localInputTask);
		centralLooper.registerFrameSyncCallback(fsync);
		//	inputHandler.registerCallback(inEvCb);
		inputHandler.registerCallback(myDot);

		graphicsManager.setRenderer(bgRenderer);
		graphicsManager.setRenderer(txtRenderer);
		//	graphicsManager.setRenderer(dotRenderer);
		graphicsManager.setRenderer(myDot);

		centralLooper.start(60);

		graphicsManager.runUntilClosed();

		centralLooper.stop();

		////////////////////////////////////////////

		delete myDot;

		delete dotRenderer;
		delete txtRenderer;
		delete bgRenderer;

		delete localInputTask;
		delete fsync;
		delete inEvCb;
	}
	_CrtDumpMemoryLeaks();

	return 0;
}