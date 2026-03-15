
#include <windows.h>
#include <Xinput.h>

#pragma comment(lib, "xinput.lib")

#include "InputHandler.h"

////////////////////////////////////
// APIs
////////////////////////////////////
InputHandler* InputHandler::getInstance() {
	if (!sInstance) {
		sInstance = new InputHandler();
	}
	return sInstance;
}

void InputHandler::destroyInstance() {
	if (sInstance) {
		delete sInstance;
		sInstance = nullptr;
	}
}

int InputHandler::update()
{
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

////////////////////////////////////
// Private
////////////////////////////////////
InputHandler* InputHandler::sInstance = nullptr;

InputHandler::InputHandler()
{
}

void InputHandler::handleXInput()
{

}

