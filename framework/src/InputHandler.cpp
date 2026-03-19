#include <windows.h>
#include <Xinput.h>
#include <string>
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
	std::lock_guard<std::mutex> lock(mMutex);
	handleXInput();
	handleKeyboard();
	return 0;
}

int InputHandler::setConf(std::string conf)
{
	Utility::printLog("InputHandler::setConf");

	std::lock_guard<std::mutex> lock(mMutex);

	mXInputMap.clear();
	mKeyboardMap.clear();

	json j = json::parse(conf);

	for (auto& [type, values] : j.items())	{
		std::string xinput   = values["xinput"];
		std::string keyboard = values["keyboard"];

		IInputHandlerCallback::InputType inputype = convTypeFromJSONEntry(type);
		mXInputMap.emplace_back(inputype, static_cast<unsigned short>(std::stoul(xinput, nullptr, 16)));
		mKeyboardMap.emplace_back(inputype, keyboard[0]);
	}
	return 0;
}

void InputHandler::registerCallback(IInputHandlerCallback* cb)
{
	mInputHandlerCallbacks.push_back(cb);
}

InputHandler::InputHandler() :
	mXInputPrevPktNum(0), mXInputPrevButtonState(0)
{
	/*
	mXInputMap.emplace_back(IInputHandlerCallback::UP,		XINPUT_GAMEPAD_DPAD_UP);
	mXInputMap.emplace_back(IInputHandlerCallback::DOWN,	XINPUT_GAMEPAD_DPAD_DOWN);
	mXInputMap.emplace_back(IInputHandlerCallback::LEFT,	XINPUT_GAMEPAD_DPAD_LEFT);
	mXInputMap.emplace_back(IInputHandlerCallback::RIGHT,	XINPUT_GAMEPAD_DPAD_RIGHT);
	mXInputMap.emplace_back(IInputHandlerCallback::ACTION1, XINPUT_GAMEPAD_A);

	mKeyboardMap.emplace_back(IInputHandlerCallback::UP,      'W');
	mKeyboardMap.emplace_back(IInputHandlerCallback::DOWN,    'S');
	mKeyboardMap.emplace_back(IInputHandlerCallback::LEFT,    'A');
	mKeyboardMap.emplace_back(IInputHandlerCallback::RIGHT,   'D');
	mKeyboardMap.emplace_back(IInputHandlerCallback::ACTION1, 'E');
	*/
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
			IInputHandlerCallback::InputState state = IInputHandlerCallback::InputState::UNKNOWN_STATE;

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

			if (state != IInputHandlerCallback::InputState::UNKNOWN_STATE) {
				for (const auto& mapElm : mXInputMap) {
					if (mapElm.second == btnBit) {
						events.push_back(std::pair<IInputHandlerCallback::InputState, IInputHandlerCallback::InputType>(state, mapElm.first));
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
		if (GetAsyncKeyState(mapElm.second) & 0x8000) {
			if (std::find(mKeyboardPrevState.begin(), mKeyboardPrevState.end(), mapElm.second) == mKeyboardPrevState.end()) {
				events.push_back(std::pair<IInputHandlerCallback::InputState, IInputHandlerCallback::InputType>(IInputHandlerCallback::PUSHED, mapElm.first));
			}
			else {
				events.push_back(std::pair<IInputHandlerCallback::InputState, IInputHandlerCallback::InputType>(IInputHandlerCallback::PRESSED, mapElm.first));
			}
			currState.push_back(mapElm.second);
		}
	}

	for (auto prevState : mKeyboardPrevState) {
		if (std::find(currState.begin(), currState.end(), prevState) == currState.end()) {
			for (const auto& mapElm : mKeyboardMap) {
				if (mapElm.second == prevState) {
					events.push_back(std::pair<IInputHandlerCallback::InputState, IInputHandlerCallback::InputType>(IInputHandlerCallback::RELEASED, mapElm.first));
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

InputHandler::IInputHandlerCallback::InputType InputHandler::convTypeFromJSONEntry(std::string type)
{
	if (type == "UP") {
		return IInputHandlerCallback::UP;
	} 

	if (type == "DOWN") {
		return IInputHandlerCallback::DOWN;
	}

	if (type == "LEFT") {
		return IInputHandlerCallback::LEFT;
	}

	if (type == "RIGHT") {
		return IInputHandlerCallback::RIGHT;
	}

	if (type == "ACTION1") {
		return IInputHandlerCallback::ACTION1;
	}

	if (type == "ACTION2") {
		return IInputHandlerCallback::ACTION2;
	}

	if (type == "ACTION3") {
		return IInputHandlerCallback::ACTION3;
	}

	if (type == "ACTION4") {
		return IInputHandlerCallback::ACTION4;
	}

	return IInputHandlerCallback::UNKNOWN_TYPE;
}



