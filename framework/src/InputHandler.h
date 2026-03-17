#pragma once

#include <mutex>
#include <vector>

#include "IInputHandler.h"

class InputHandler : public IInputHandler {
public:
	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	int  update();
	int  setConf(std::string conf);
	void registerCallback(IInputHandlerCallback* cb);

	InputHandler();
	virtual ~InputHandler() = default;
private:
	void handleXInput();
	void handleKeyboard();

	std::mutex		   mMutex;

	std::vector<IInputHandlerCallback*>
		mInputHandlerCallbacks;

	unsigned int	mXInputPrevPktNum;
	unsigned short	mXInputPrevButtonState;
};