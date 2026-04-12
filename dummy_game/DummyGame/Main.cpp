#include <memory>
#include <mutex>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <thread>
#include <condition_variable>

#include "RoseAura.h"
#include "RoseAuraReturnCode.h"
#include "MediaUtility.h"

#include "Utility.h"
#include "raylib.h"

#include "CommonObjects.h"
#include "GameObjects.h"
#include "OpeningObjects.h"

using namespace RoseAuraMediaUtility;
using namespace RoseAuraReturnCode;

#define WIN_SIZE_W 800
#define WIN_SIZE_H 600

////////////////////////////////////////////
////////////////////////////////////////////
std::string readInputConf()
{
	std::ifstream file("input_map.json");

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
		IObjectRepository& objectRepository = rose_aura->getObjectRepository();
		ISoundCoordinator& soundCoordinator = rose_aura->getSoundCoordinator();

		////////////////////////////////////////////
		GameObjects::buildWorldConf();
		IWorldNavigator::WORLD_ID wId = worldNavigator.createWorld(GameObjects::gWorldConf);

		std::vector<IObjectRepository::OBJECT_ID> commonObjIDs 
			= CommonObjects::registerObjects(*rose_aura);

		std::vector<IObjectRepository::OBJECT_ID> gameObjIDs
			= GameObjects::registerObjects(*rose_aura);

		std::vector<IObjectRepository::OBJECT_ID> OpeningObjIDs
			= OpeningObjects::registerObjects(*rose_aura);

		inputHandler.setConf(readInputConf());

		graphicsManager.setShaderFile(OpeningObjects::CONVERT_PICTURE_SHADER);

		////////////////////////////////////////////

		objectRepository.activateByTag(CommonObjects::TAG_COMMON_OBJECT);
		objectRepository.activateByTag(GameObjects::TAG_GAME_OBJECT);

		////////////////////////////////////////////
		soundCoordinator.start();
		centralLooper.start(30);
		graphicsManager.runUntilClosed();

		////////////////////////////////////////////
		centralLooper.stop();
		soundCoordinator.stop();
		objectRepository.deactivateByTag(GameObjects::TAG_GAME_OBJECT);
		objectRepository.deactivateByTag(CommonObjects::TAG_COMMON_OBJECT);

	}
	_CrtDumpMemoryLeaks();

	return 0;
}