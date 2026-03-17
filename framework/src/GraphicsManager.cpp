#include "GraphicsManager.h"
#include "Utility.h"

#include "raylib.h"
 
////////////////////////////////////
// APIs
////////////////////////////////////
void GraphicsManager::runUntilClosed()
{
	while (!WindowShouldClose())
	{

		Utility::printLog("runUntilClosed lock in");

		BeginDrawing();

		mMutex.lock();
		std::vector<IObjectRenderer*> renderers = mRenderers;
		mMutex.unlock();

		for (IObjectRenderer* renderer : renderers)
		{
			renderer->render();
		}

		EndDrawing();

		Utility::printLog("runUntilClosed lock out");
	}

	CloseWindow();
}

void GraphicsManager::setRenderer(IObjectRenderer* renderer)
{
	Utility::printLog("setRenderer in");
	mMutex.lock();
	mRenderers.push_back(renderer);
	mMutex.unlock();
	Utility::printLog("setRenderer in");
}

void GraphicsManager::removeRenderer(IObjectRenderer* renderer)
{
	Utility::printLog("removeRenderer in");
	mMutex.lock();
	auto target = std::find(mRenderers.begin(), mRenderers.end(), renderer);
	if (target != mRenderers.end())
	{
		mRenderers.erase(target);
	}
	mMutex.unlock();
	Utility::printLog("removeRenderer out");
}

GraphicsManager::GraphicsManager()
{
	InitWindow(800, 450, "Rose Aura");
	SetTargetFPS(60);
}

////////////////////////////////////
// Private
////////////////////////////////////

