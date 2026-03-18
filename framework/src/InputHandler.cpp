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
	handleKeyboard();
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
	mXInputMap.emplace_back(XINPUT_GAMEPAD_DPAD_UP		, IInputHandlerCallback::UP);
	mXInputMap.emplace_back(XINPUT_GAMEPAD_DPAD_DOWN	, IInputHandlerCallback::DOWN);
	mXInputMap.emplace_back(XINPUT_GAMEPAD_DPAD_LEFT	, IInputHandlerCallback::LEFT);
	mXInputMap.emplace_back(XINPUT_GAMEPAD_DPAD_RIGHT	, IInputHandlerCallback::RIGHT);
	mXInputMap.emplace_back(XINPUT_GAMEPAD_A			, IInputHandlerCallback::ACTION1);

	mKeyboardMap.emplace_back('W', IInputHandlerCallback::UP);
	mKeyboardMap.emplace_back('S', IInputHandlerCallback::DOWN);
	mKeyboardMap.emplace_back('A', IInputHandlerCallback::LEFT);
	mKeyboardMap.emplace_back('D', IInputHandlerCallback::RIGHT);
	mKeyboardMap.emplace_back('E', IInputHandlerCallback::ACTION1);

}

////////////////////////////////////
// Private
////////////////////////////////////
void InputHandler::handleXInput()
{
	XINPUT_STATE xInputCurrState{};

	std::vector<std::pair<IInputHandlerCallback::InputState, IInputHandlerCallback::InputType>> events;

	if (XInputGetState(0, &xInputCurrState) != ERROR_SUCCESS)
	{
		ZeroMemory(&xInputCurrState, sizeof(XINPUT_STATE));
	}

	if (xInputCurrState.dwPacketNumber != mXInputPrevPktNum || 0 != mXInputPrevButtonState)
	{
		for (unsigned short btnBit = 0x0001; btnBit > 0; btnBit = (btnBit << 1)) {
			IInputHandlerCallback::InputState state = IInputHandlerCallback::InputState::UNKNOWN;

			unsigned short curr = xInputCurrState.Gamepad.wButtons & btnBit;
			unsigned short prev = mXInputPrevButtonState & btnBit;

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
				for (const auto& mapElm : mXInputMap) {
					if (mapElm.first == btnBit) {
						events.push_back(std::pair<IInputHandlerCallback::InputState, IInputHandlerCallback::InputType>(state, mapElm.second));
					}
				}

			}
		}
	}

	if (!events.empty()) {
		doCallback(events);
	}

	mXInputPrevPktNum      = xInputCurrState.dwPacketNumber;
	mXInputPrevButtonState = xInputCurrState.Gamepad.wButtons;
}

void InputHandler::handleKeyboard()
{
	std::vector<std::pair<IInputHandlerCallback::InputState, IInputHandlerCallback::InputType>> events;

	std::vector<char> currState;

	for (auto mapElm : mKeyboardMap) {
		if (GetAsyncKeyState(mapElm.first) & 0x8000) {
			if (std::find(mKeyboardPrevState.begin(), mKeyboardPrevState.end(), mapElm.first) == mKeyboardPrevState.end()) {
				events.push_back(std::pair<IInputHandlerCallback::InputState, IInputHandlerCallback::InputType>(IInputHandlerCallback::PUSHED, mapElm.second));
			}
			else {
				events.push_back(std::pair<IInputHandlerCallback::InputState, IInputHandlerCallback::InputType>(IInputHandlerCallback::PRESSED, mapElm.second));
			}
			currState.push_back(mapElm.first);
		}
	}

	for (auto prevState : mKeyboardPrevState) {
		if (std::find(currState.begin(), currState.end(), prevState) == currState.end()) {
			for (const auto& mapElm : mKeyboardMap) {
				if (mapElm.first == prevState) {
					events.push_back(std::pair<IInputHandlerCallback::InputState, IInputHandlerCallback::InputType>(IInputHandlerCallback::RELEASED, mapElm.second));
				}
			}
		}
	}

	if (!events.empty()) {
		doCallback(events);
	}

	mKeyboardPrevState = currState;
}

void InputHandler::doCallback(std::vector<std::pair<IInputHandlerCallback::InputState, IInputHandlerCallback::InputType>>& events)
{
	for (IInputHandlerCallback* frameSyncCallback : mInputHandlerCallbacks) {
		frameSyncCallback->onEvent(events);
	}
}



