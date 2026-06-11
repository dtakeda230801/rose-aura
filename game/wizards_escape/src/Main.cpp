#include <memory>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "RoseAura.h"

#include "Utility.h"

#include "CommonObjects.h"
#include "OpeningObjects.h"
#include "TitleObjects.h"
#include "GameObjects.h"

#include "Game.h"

using namespace RoseAuraMediaUtility;

////////////////////////////////////////////
////////////////////////////////////////////
std::string readInputConf()
{
	std::ifstream file("resources\\input_map.json");

	if (!file) {
		Utility::printLog("can not read input_map.json");
		return "";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();

	return buffer.str();
}


////////////////////////////////////////////
FwHolder* FwHolder::mInstance = nullptr;

////////////////////////////////////////////
////////////////////////////////////////////
int main()
{
	{ //for _CrtDumpMemoryLeaks()
		////////////////////////////////////////////
		FwHolder::create();
		
		////////////////////////////////////////////
		GameObjects::buildWorldConf();
		IWorldNavigator::WORLD_ID wId = RA_WORLD_NAVIGATOR.createWorld(GameObjects::gWorldConf);
		
		std::vector<IObjectActivator::OBJECT_ID> commonObjIDs 
			= CommonObjects::registerObjects();

		std::vector<IObjectActivator::OBJECT_ID> OpeningObjIDs
			= OpeningObjects::registerObjects();

		std::vector<IObjectActivator::OBJECT_ID> TitleObjIDs
			= TitleObjects::registerObjects();

		std::vector<IObjectActivator::OBJECT_ID> gameObjIDs
			= GameObjects::registerObjects();


		RA_INPUT_HANDLER.setConf(readInputConf());

		IGraphicsManager::Conf conf;
		conf.mWindowWidth  = WIN_SIZE_W; 
		conf.mWindowHeight = WIN_SIZE_H;
		conf.mWindowTitle  = WIN_TITLE;
		conf.mFrameRate    = VIDEO_FRAME_RATE;

		////////////////////////////////////////////

		RA_OBJECT_ACTIVATOR.activateByTag(TAG_COMMON_OBJECT);
		//RA_OBJECT_ACTIVATOR.activateByTag(TAG_GAME_OBJECT);
		//RA_OBJECT_ACTIVATOR.activateByTag(TAG_OPENING_OBJECT);

		////////////////////////////////////////////
		RA_SOUND_COORDINATOR.start();
		RA_CENTRAL_LOOPER.start(LOOPER_FRAME_RATE);
		RA_GRAPHICS_MANAGER.runUntilClosed(conf);

		////////////////////////////////////////////
		RA_CENTRAL_LOOPER.stop();
		RA_SOUND_COORDINATOR.stop();
		RA_OBJECT_ACTIVATOR.deactivateByTag(TAG_GAME_OBJECT);
		RA_OBJECT_ACTIVATOR.deactivateByTag(TAG_OPENING_OBJECT);
		RA_OBJECT_ACTIVATOR.deactivateByTag(TAG_COMMON_OBJECT);

		FwHolder::destroy();
	}
	_CrtDumpMemoryLeaks();

	return 0;
}