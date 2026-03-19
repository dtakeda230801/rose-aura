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
	void doCallback(std::vector<std::pair<IInputHandlerCallback::InputState,IInputHandlerCallback::InputType>>& events);

	IInputHandlerCallback::InputType
		convTypeFromJSONEntry(std::string);

	std::mutex		   mMutex;

	std::vector<IInputHandlerCallback*>
		mInputHandlerCallbacks;

	std::vector<std::pair<IInputHandlerCallback::InputType, unsigned short>> mXInputMap;

	unsigned int	mXInputPrevPktNum;
	unsigned short	mXInputPrevButtonState;

	std::vector<std::pair<IInputHandlerCallback::InputType, char>>			 mKeyboardMap;

	std::vector<char> mKeyboardPrevState;
};