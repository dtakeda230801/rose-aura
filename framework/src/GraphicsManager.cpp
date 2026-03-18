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
		BeginDrawing();

		mMutex.lock();
		std::vector<IObjectRenderer*> renderers = mRenderers;
		mMutex.unlock();

		for (IObjectRenderer* renderer : renderers)
		{
			renderer->render();
		}

		EndDrawing();
	}

	CloseWindow();
}

void GraphicsManager::setRenderer(IObjectRenderer* renderer)
{
	mMutex.lock();
	mRenderers.push_back(renderer);
	mMutex.unlock();
}

void GraphicsManager::removeRenderer(IObjectRenderer* renderer)
{
	mMutex.lock();
	auto target = std::find(mRenderers.begin(), mRenderers.end(), renderer);
	if (target != mRenderers.end())
	{
		mRenderers.erase(target);
	}
	mMutex.unlock();
}

GraphicsManager::GraphicsManager()
{
	InitWindow(800, 450, "Rose Aura");
	SetTargetFPS(60);
}

////////////////////////////////////
// Private
////////////////////////////////////

