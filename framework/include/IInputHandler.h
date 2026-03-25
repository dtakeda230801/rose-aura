#pragma once

#include <string>
#include <vector>

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

	protected:
		IInputHandlerCallback() = default;
		virtual ~IInputHandlerCallback() = default;
	};

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	virtual int update()	= 0;
	virtual int setConf(std::string conf)	= 0;
	virtual int registerCallback(IInputHandlerCallback* cb) = 0;
	virtual int unregisterCallback(IInputHandlerCallback* cb) = 0;
};