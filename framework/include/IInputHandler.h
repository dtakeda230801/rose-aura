#pragma once

class IInputHandler {
public:
	class IInputHandlerCallback {
	public:
		typedef enum {
			Down,
			Pressed,
			Release,
			Unknown
		} Action;

		virtual void onEvent(Action action, unsigned short pushed) = 0;

	protected:
		IInputHandlerCallback() = default;
		virtual ~IInputHandlerCallback() = default;
	};

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	virtual int  update()	= 0;
	virtual int  setConf()	= 0;
	virtual void registerCallback(IInputHandlerCallback* cb) = 0;
};