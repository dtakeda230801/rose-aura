#pragma once

#include <memory>
#include <mutex>
#include <vector>

class InputHandler {
public:
	class IInputHandlerCallback {
	public:
		typedef enum {
			Down,
			Pressed,
			Release,
			Unknown
		} Action;

		virtual void onEvent(Action action,unsigned short pushed) = 0;

	protected:
		IInputHandlerCallback()          = default;
		virtual ~IInputHandlerCallback() = default;
	};

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	static void createInstance();
	static void destroyInstance();
	static InputHandler& getInstance();

	int  update();
	int  setConf();
	void registerCallback(IInputHandlerCallback* cb);

	virtual ~InputHandler() = default;
private:
	static std::unique_ptr<InputHandler> mInstance;

	InputHandler();

	void handleXInput();

	std::mutex		   mMutex;

	std::vector<IInputHandlerCallback*>
		mInputHandlerCallbacks;

	unsigned int	mXInputPrevPktNum;
	unsigned short	mXInputPrevButtonState;
};