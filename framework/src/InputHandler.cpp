#include <windows.h>
#include <Xinput.h>
#include "InputHandler.h"
#include "Utility.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

#pragma comment(lib, "xinput.lib")


////////////////////////////////////
// APIs
////////////////////////////////////
int InputHandler::update()
{
	handleXInput();
	return 0;
}

int InputHandler::setConf(std::string conf)
{
	json j = json::parse(conf);

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
			IInputHandlerCallback::InputState state = IInputHandlerCallback::InputState::UNKNOWN;

			unsigned int curr = mXInputCurrState.Gamepad.wButtons & btnBit;
			unsigned int prev = mXInputPrevButtonState & btnBit;

			if (0 == prev && 0 != curr) {
				state = IInputHandlerCallback::InputState::PUSHED;
			}

			if (0 != prev && 0 != curr) {
				state = IInputHandlerCallback::InputState::PRESSED;
			}

			if (0 != prev && 0 == curr) {
				state = IInputHandlerCallback::InputState::RELEASED;
			}

			if (state != IInputHandlerCallback::InputState::UNKNOWN) {
				std::vector<IInputHandlerCallback::InputType> types;

				convXInputType(btnBit, types);

				for (IInputHandlerCallback* frameSyncCallback : mInputHandlerCallbacks) {
					frameSyncCallback->onEvent(state, types);
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

void InputHandler::convXInputType(unsigned short in, std::vector<IInputHandlerCallback::InputType>& out)
{
	out.clear();

	if (in & XINPUT_GAMEPAD_DPAD_UP) {
		out.push_back(IInputHandlerCallback::UP);
	}

	if (in & XINPUT_GAMEPAD_DPAD_DOWN) {
		out.push_back(IInputHandlerCallback::DOWN);
	}

	if (in & XINPUT_GAMEPAD_DPAD_LEFT) {
		out.push_back(IInputHandlerCallback::LEFT);
	}

	if (in & XINPUT_GAMEPAD_DPAD_RIGHT) {
		out.push_back(IInputHandlerCallback::RIGHT);
	}

	if (in & XINPUT_GAMEPAD_A) {
		out.push_back(IInputHandlerCallback::ACTION1);
	}

	if (in & XINPUT_GAMEPAD_B) {
		out.push_back(IInputHandlerCallback::ACTION2);
	}

	if (in & XINPUT_GAMEPAD_X) {
		out.push_back(IInputHandlerCallback::ACTION3);
	}

	if (in & XINPUT_GAMEPAD_Y) {
		out.push_back(IInputHandlerCallback::ACTION4);
	}
}
