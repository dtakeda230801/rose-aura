#include "raylib.h"

#include "GraphicsManager.h"
#include "Utility.h"


////////////////////////////////////
// APIs
////////////////////////////////////
void GraphicsManager::runUntilClosed(Conf conf)
{
	mRunning.store(true);

	InitWindow(conf.mWindowWidth, conf.mWindowHeight, conf.mWindowTitle);
	SetTargetFPS(conf.mFrameRate);

	while (!WindowShouldClose() && mRunning)
	{
		for (auto& holder : mShaderHolders) {
			if (!holder.mShader) {
				Shader* shader = new Shader();
				if (holder.mIsFile) {
					*(shader) = LoadShader(holder.mVShader.c_str(), holder.mFShader.c_str());
				}
				else {
					*(shader) = LoadShaderFromMemory(holder.mVShader.c_str(), holder.mFShader.c_str());
				}
				holder.mShader = static_cast<void*>(shader);
			}
		}

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

void GraphicsManager::exit()
{
	mRunning.store(false);
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
	SHADER_ID newId = static_cast<SHADER_ID>(mShaderHolders.size() + 1);
	mShaderHolders.emplace_back(ShaderHolder{ nullptr, newId, vsfile, fsfile , true });
	id = newId;
	return RARetCode::RET_OK;
}

RARetCode GraphicsManager::setShader(std::string vsString
	                               , std::string fsString
	                               , SHADER_ID& id)
{
	SHADER_ID newId = static_cast<SHADER_ID>(mShaderHolders.size() + 1);
	mShaderHolders.emplace_back(ShaderHolder{ nullptr, newId, vsString, fsString , false });
	id = newId;
	return RARetCode::RET_OK;

}


void* GraphicsManager::getShader(SHADER_ID id)
{
	for (auto& shaderHolder : mShaderHolders) {
		if (shaderHolder.mId == id) {
			if (shaderHolder.mShader) {
				return shaderHolder.mShader;
			} else {
				Utility::printLog("Shader not readly");
				return nullptr;
			}
		}
	}
	return nullptr;
}

////////////////////////////////////
// Private
////////////////////////////////////

