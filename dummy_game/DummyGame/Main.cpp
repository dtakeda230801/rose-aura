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

#include "DummyGame.h"

using namespace RoseAuraMediaUtility;
using namespace RoseAuraReturnCode;


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
		IObjectActivator& objectRepository = rose_aura->getObjectRepository();
		ISoundCoordinator& soundCoordinator = rose_aura->getSoundCoordinator();

		////////////////////////////////////////////
		GameObjects::buildWorldConf();
		IWorldNavigator::WORLD_ID wId = worldNavigator.createWorld(GameObjects::gWorldConf);

		std::vector<IObjectActivator::OBJECT_ID> commonObjIDs 
			= CommonObjects::registerObjects(*rose_aura);

		std::vector<IObjectActivator::OBJECT_ID> gameObjIDs
			= GameObjects::registerObjects(*rose_aura);

		std::vector<IObjectActivator::OBJECT_ID> OpeningObjIDs
			= OpeningObjects::registerObjects(*rose_aura);

		inputHandler.setConf(readInputConf());

		graphicsManager.setShaderFile(OpeningObjects::CONVERT_PICTURE_V_SHADER
			                        , OpeningObjects::CONVERT_PICTURE_F_SHADER
		                            , OpeningObjects::OPENING_SHADER_ID);

		IGraphicsManager::Conf conf;
		conf.mWindowWidth  = WIN_SIZE_W; 
		conf.mWindowHeight = WIN_SIZE_H;
		conf.mWindowTitle  = WIN_TITLE;
		conf.mFrameRate    = VIDEO_FRAME_RATE;

		////////////////////////////////////////////

		objectRepository.activateByTag(TAG_COMMON_OBJECT);
		objectRepository.activateByTag(TAG_GAME_OBJECT);
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