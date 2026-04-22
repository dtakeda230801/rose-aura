#include "raylib.h"

#include "GraphicsManager.h"
#include "Utility.h"


////////////////////////////////////
// APIs
////////////////////////////////////
void GraphicsManager::runUntilClosed(Conf conf)
{
	InitWindow(conf.mWindowWidth, conf.mWindowHeight, conf.mWindowTitle);
	SetTargetFPS(conf.mFrameRate);

	for (auto& holder : mShaderHolders) {
		Shader* shader = static_cast<Shader*>(holder.mShader);
		*(shader)      = LoadShader(holder.mVShaderFile.c_str(), holder.mFShaderFile.c_str());
	}

	while (!WindowShouldClose())
	{
		mMutex.lock();
		std::vector<IGraphicsRenderer*> renderers = mRenderers;
		mMutex.unlock();

		for (IGraphicsRenderer* renderer : renderers)
		{
			renderer->preprocess();
		}

		BeginDrawing();

		for (IGraphicsRenderer* renderer : renderers)
		{
			renderer->render();
		}

		EndDrawing();
	}

	for (auto& holder : mShaderHolders) {
		Shader* shader = static_cast<Shader*>(holder.mShader);
		UnloadShader(*shader);
		delete shader;
	}

	CloseWindow();
}

RARetCode GraphicsManager::setRenderer(IGraphicsRenderer* renderer)
{	
	if (!renderer) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}

	mMutex.lock();
	mRenderers.push_back(renderer);
	mMutex.unlock();

	return RARetCode::RET_OK;
}

RARetCode GraphicsManager::removeRenderer(IGraphicsRenderer* renderer)
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

RARetCode GraphicsManager::setShaderFile(std::string vsfile
									   , std::string fsfile
									   , SHADER_ID& id)
{	
	SHADER_ID newId = mShaderHolders.size() + 1;
	mShaderHolders.emplace_back(ShaderHolder{ static_cast<void*>(new Shader()),newId, vsfile, fsfile });
	id = newId;
	return RARetCode::RET_OK;
}

void* GraphicsManager::getShader(SHADER_ID id)
{
	for (auto& shaderHolder : mShaderHolders) {
		if (shaderHolder.mId == id) {
			return shaderHolder.mShader;
		}
	}
	return nullptr;
}

////////////////////////////////////
// Private
////////////////////////////////////

