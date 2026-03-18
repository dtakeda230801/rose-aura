#pragma once

#include <string>
#include <vector>

class IInputHandler {
public:
	class IInputHandlerCallback {
	public:
		typedef enum {
			PUSHED,
			PRESSED,
			RELEASED,
			UNKNOWN
		} InputState;

		typedef enum {
			UP      = 0x01,
			DOWN	= 0x02, 
			LEFT	= 0x03, 
			RIGHT	= 0x04, 
			ACTION1 = 0x05,
			ACTION2 = 0x06,
			ACTION3 = 0x07,
			ACTION4 = 0x08
		} InputType;

		virtual void onEvent(std::vector<std::pair<InputState,InputType>>& events) = 0;

	protected:
		IInputHandlerCallback() = default;
		virtual ~IInputHandlerCallback() = default;
	};

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	virtual int  update()	= 0;
	virtual int  setConf(std::string conf)	= 0;
	virtual void registerCallback(IInputHandlerCallback* cb) = 0;
};