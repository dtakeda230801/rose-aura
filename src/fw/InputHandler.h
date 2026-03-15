#pragma once

#include <mutex>
#include <vector>

class InputHandler {
public:
	class IInputHandlerCallback {
	public:
		virtual void onEvent() = 0;

	protected:
		IInputHandlerCallback()          = default;
		virtual ~IInputHandlerCallback() = default;
	};

	static InputHandler* getInstance();
	static void destroyInstance();

	int  update();
	int  setConf();
	void registerCallback(IInputHandlerCallback* cb);

private:
	static InputHandler* sInstance;

	InputHandler();
	virtual ~InputHandler() = default;

	void handleXInput();

	std::mutex		   mMutex;

	std::vector<IInputHandlerCallback*>
		mInputHandlerCallbacks;

};