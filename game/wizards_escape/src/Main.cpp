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
////////////////////////////////////////////
int main()
{
	{ //for _CrtDumpMemoryLeaks()
		////////////////////////////////////////////
		std::unique_ptr<RoseAura> rose_aura = RoseAura::create();

		ICentralLooper&	   centralLooper	= rose_aura->getCentralLooper();
		IInputHandler&	   inputHandler	    = rose_aura->getInputHandler();
		IGraphicsManager&  graphicsManager  = rose_aura->getGraphicsManager();
		IWorldNavigator&   worldNavigator   = rose_aura->getWorldNavigator();
		IObjectActivator&  objectRepository = rose_aura->getObjectActivator();
		ISoundCoordinator& soundCoordinator = rose_aura->getSoundCoordinator();
		
		////////////////////////////////////////////
		GameObjects::buildWorldConf();
		IWorldNavigator::WORLD_ID wId = worldNavigator.createWorld(GameObjects::gWorldConf);
		
		std::vector<IObjectActivator::OBJECT_ID> commonObjIDs 
			= CommonObjects::registerObjects(*rose_aura);

		std::vector<IObjectActivator::OBJECT_ID> OpeningObjIDs
			= OpeningObjects::registerObjects(*rose_aura);

		std::vector<IObjectActivator::OBJECT_ID> TitleObjIDs
			= TitleObjects::registerObjects(*rose_aura);

		std::vector<IObjectActivator::OBJECT_ID> gameObjIDs
			= GameObjects::registerObjects(*rose_aura);


		inputHandler.setConf(readInputConf());

		IGraphicsManager::Conf conf;
		conf.mWindowWidth  = WIN_SIZE_W; 
		conf.mWindowHeight = WIN_SIZE_H;
		conf.mWindowTitle  = WIN_TITLE;
		conf.mFrameRate    = VIDEO_FRAME_RATE;

		////////////////////////////////////////////

		objectRepository.activateByTag(TAG_COMMON_OBJECT);
		//objectRepository.activateByTag(TAG_GAME_OBJECT);
		//objectRepository.activateByTag(TAG_OPENING_OBJECT);

		////////////////////////////////////////////
		soundCoordinator.start();
		centralLooper.start(LOOPER_FRAME_RATE);
		graphicsManager.runUntilClosed(conf);

		////////////////////////////////////////////
		centralLooper.stop();
		soundCoordinator.stop();
		objectRepository.deactivateByTag(TAG_GAME_OBJECT);
		objectRepository.deactivateByTag(TAG_OPENING_OBJECT);
		objectRepository.deactivateByTag(TAG_COMMON_OBJECT);
	}
	_CrtDumpMemoryLeaks();

	return 0;
}