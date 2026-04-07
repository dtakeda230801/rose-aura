#pragma once

#include <string>
#include <vector>
#include "RoseAuraReturnCode.h"

using namespace RoseAuraReturnCode;

class IInputHandler {
public:
	class IInputHandlerCallback {
	public:
		enum class InputState {
			PUSHED,
			PRESSED,
			RELEASED,
			UNKNOWN_STATE
		};

		enum class InputType {
			UP			 = 0x01,
			DOWN		 = 0x02, 
			LEFT		 = 0x03, 
			RIGHT		 = 0x04, 
			ACTION1		 = 0x05,
			ACTION2		 = 0x06,
			ACTION3		 = 0x07,
			ACTION4		 = 0x08,
			UNKNOWN_TYPE = 0xFF
		};

		virtual void onEvent(std::vector<std::pair<InputState,InputType>>& events) = 0;

		virtual ~IInputHandlerCallback() = default;
	protected:
		IInputHandlerCallback() = default;
	};

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	virtual void      update() = 0;
	virtual RARetCode setConf(std::string conf) = 0;
	virtual RARetCode registerCallback(IInputHandlerCallback* cb) = 0;
	virtual RARetCode unregisterCallback(IInputHandlerCallback* cb) = 0;
};