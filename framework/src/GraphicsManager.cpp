#include "GraphicsManager.h"
#include "Utility.h"

#include "raylib.h"
 
////////////////////////////////////
// APIs
////////////////////////////////////
void GraphicsManager::runUntilClosed()
{
	InitWindow(800, 450, "Rose Aura");
	SetTargetFPS(60);

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

RARetCode GraphicsManager::setRenderer(IObjectRenderer* renderer)
{	
	if (!renderer) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}

	mMutex.lock();
	mRenderers.push_back(renderer);
	mMutex.unlock();

	return RARetCode::RET_OK;
}

RARetCode GraphicsManager::removeRenderer(IObjectRenderer* renderer)
{
	if (!renderer) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}
			
	RARetCode ret = RARetCode::RET_OK;

	mMutex.lock();
	if (0 != Utility::eraseVectorElm(mRenderers, renderer)) {
		ret = RARetCode::RET_ERR_INVALID_ARG;
	}
	mMutex.unlock();

	return ret;
}

////////////////////////////////////
// Private
////////////////////////////////////

