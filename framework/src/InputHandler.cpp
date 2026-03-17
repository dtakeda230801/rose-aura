#include <windows.h>
#include <Xinput.h>
#include "InputHandler.h"
#include "Utility.h"

#pragma comment(lib, "xinput.lib")


////////////////////////////////////
// APIs
////////////////////////////////////
int InputHandler::update()
{
	Utility::printLog("InputHandler#update");
	handleXInput();
	return 0;
}

int InputHandler::setConf()
{
	return 0;
}

void InputHandler::registerCallback(IInputHandlerCallback* cb)
{
	mInputHandlerCallbacks.push_back(cb);
}

InputHandler::InputHandler() :
	mXInputPrevPktNum(0), mXInputPrevButtonState(0)
{
}

////////////////////////////////////
// Private
////////////////////////////////////
void InputHandler::handleXInput()
{
	XINPUT_STATE mXInputCurrState{};

	if (XInputGetState(0, &mXInputCurrState) != ERROR_SUCCESS)
	{
		ZeroMemory(&mXInputCurrState, sizeof(XINPUT_STATE));
	}

	if (mXInputCurrState.dwPacketNumber != mXInputPrevPktNum || 0 != mXInputPrevButtonState)
	{
		for (unsigned int btnBit = 0x0001; btnBit > 0; btnBit = (btnBit << 1)) {
			IInputHandlerCallback::Action action = IInputHandlerCallback::Action::Unknown;

			unsigned int curr = mXInputCurrState.Gamepad.wButtons & btnBit;
			unsigned int prev = mXInputPrevButtonState & btnBit;

			if (0 == prev && 0 != curr) {
				action = IInputHandlerCallback::Action::Down;
			}

			if (0 != prev && 0 != curr) {
				action = IInputHandlerCallback::Action::Pressed;
			}

			if (0 != prev && 0 == curr) {
				action = IInputHandlerCallback::Action::Release;
			}

			if (action != IInputHandlerCallback::Action::Unknown) {
				for (IInputHandlerCallback* frameSyncCallback : mInputHandlerCallbacks) {
					frameSyncCallback->onEvent(action, btnBit);
				}
			}

		}
	}

	mXInputPrevPktNum = mXInputCurrState.dwPacketNumber;
	mXInputPrevButtonState = mXInputCurrState.Gamepad.wButtons;
}

void InputHandler::handleKeyboard()
{

}
