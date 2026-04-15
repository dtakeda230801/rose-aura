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
void InputHandler::update()
{
	std::lock_guard<std::mutex> lock(mMutex);
	handleXInput();
	handleKeyboard();
}

RARetCode InputHandler::setConf(std::string conf)
{
	std::lock_guard<std::mutex> lock(mMutex);

	mXInputMap.clear();
	mKeyboardMap.clear();

	json j;

	try {
		j = json::parse(conf);
	}
	catch (const json::parse_error& e) {
		Utility::printLog("json perse error:%s", e.what());
		return RARetCode::RET_ERR_INVALID_ARG;
	}

	for (auto& [type, values] : j.items())	{
		std::string xinput   = values["xinput"];
		std::string keyboard = values["keyboard"];

		IInputHandlerCallback::InputType inputype = convTypeFromJSONEntry(type);
		mXInputMap.emplace_back(inputype, static_cast<unsigned short>(std::stoul(xinput, nullptr, 16)));
		mKeyboardMap.emplace_back(inputype, keyboard[0]);
	}
	return RARetCode::RET_OK;
}

RARetCode InputHandler::registerCallback(IInputHandlerCallback* cb)
{
	if (!cb) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}

	std::lock_guard<std::mutex> lock(mMutex);
	mInputHandlerCallbacks.push_back(cb);
	return RARetCode::RET_OK;
}

RARetCode InputHandler::unregisterCallback(IInputHandlerCallback* cb)
{
	if (!cb) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}

	std::lock_guard<std::mutex> lock(mMutex);
	if (0 != Utility::eraseVectorElm(mInputHandlerCallbacks,cb)) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}
	return RARetCode::RET_OK;
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
				events.push_back(std::pair<IInputHandlerCallback::InputState, IInputHandlerCallback::InputType>(IInputHandlerCallback::InputState::PUSHED, mapElm.first));
			}
			else {
				events.push_back(std::pair<IInputHandlerCallback::InputState, IInputHandlerCallback::InputType>(IInputHandlerCallback::InputState::PRESSED, mapElm.first));
			}
			currState.push_back(mapElm.second);
		}
	}

	for (auto prevState : mKeyboardPrevState) {
		if (std::find(currState.begin(), currState.end(), prevState) == currState.end()) {
			for (const auto& mapElm : mKeyboardMap) {
				if (mapElm.second == prevState) {
					events.push_back(std::pair<IInputHandlerCallback::InputState, IInputHandlerCallback::InputType>(IInputHandlerCallback::InputState::RELEASED, mapElm.first));
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
		frameSyncCallback->onInputEvent(events);
	}
}

InputHandler::IInputHandlerCallback::InputType InputHandler::convTypeFromJSONEntry(std::string type)
{
	if (type == "UP") {
		return IInputHandlerCallback::InputType::UP;
	} 

	if (type == "DOWN") {
		return IInputHandlerCallback::InputType::DOWN;
	}

	if (type == "LEFT") {
		return IInputHandlerCallback::InputType::LEFT;
	}

	if (type == "RIGHT") {
		return IInputHandlerCallback::InputType::RIGHT;
	}

	if (type == "ACTION1") {
		return IInputHandlerCallback::InputType::ACTION1;
	}

	if (type == "ACTION2") {
		return IInputHandlerCallback::InputType::ACTION2;
	}

	if (type == "ACTION3") {
		return IInputHandlerCallback::InputType::ACTION3;
	}

	if (type == "ACTION4") {
		return IInputHandlerCallback::InputType::ACTION4;
	}

	return IInputHandlerCallback::InputType::UNKNOWN_TYPE;
}



