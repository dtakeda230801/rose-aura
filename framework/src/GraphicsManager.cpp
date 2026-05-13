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

		for (auto& holder : mFontHolders) {
			if (!holder.mFont) {
				Font* font = new Font();
				*(font) = LoadFont(holder.mFontFile.c_str());
				holder.mFont = static_cast<void*>(font);
			}
		}

		for (auto& holder : mModelHolders) {
			if (!holder.mModel) {
				Model* model = new Model();
				*(model) = LoadModel(holder.mModelFile.c_str());
				holder.mModel = static_cast<void*>(model);
			}
		}


		mMutex.lock();
		std::vector<IGraphicsRenderer*> renderers = mRenderers;
		mMutex.unlock();

		for (IGraphicsRenderer* renderer : renderers) {
			renderer->preprocess();
		}

		BeginDrawing();

		for (IGraphicsRenderer* renderer : renderers) {
			renderer->render();
		}

		EndDrawing();

		for (auto ite = mModelHolders.begin(); ite != mModelHolders.end(); ) {
			ModelHolder holder = *ite;

			if (holder.mRemove) {
				Model* model = static_cast<Model*>(holder.mModel);
				if (model) {
					UnloadModel(*model);
					delete model;
				}
				ite = mModelHolders.erase(ite);
			} else {
				++ite;
			}
		}

		for (auto ite = mFontHolders.begin(); ite != mFontHolders.end(); ) {
			FontHolder holder = *ite;

			if (holder.mRemove) {
				Font* font = static_cast<Font*>(holder.mFont);
				if (font) {
					UnloadFont(*font);
					delete font;
				}
				ite = mFontHolders.erase(ite);
			}
			else {
				++ite;
			}
		}

		for (auto ite = mShaderHolders.begin(); ite != mShaderHolders.end(); ) {
			ShaderHolder holder = *ite;

			if (holder.mRemove) {
				Shader* shader = static_cast<Shader*>(holder.mShader);
				if (shader) {
					UnloadShader(*shader);
					delete shader;
				}
				ite = mShaderHolders.erase(ite);
			}
			else {
				++ite;
			}
		}
	}
	CloseWindow();


	for (auto& holder : mModelHolders) {
		Model* model = static_cast<Model*>(holder.mModel);
		if (model){
			UnloadModel(*model);
			delete model;
		}
	}

	for (auto& holder : mFontHolders) {
		Font* font = static_cast<Font*>(holder.mFont);
		if (font) {
			UnloadFont(*font);
			delete font;
		}
	}

	for (auto& holder : mShaderHolders) {
		Shader* shader = static_cast<Shader*>(holder.mShader);
		if (shader) {
			UnloadShader(*shader);
			delete shader;
		}
	}
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
	mShaderHolders.emplace_back(ShaderHolder{ nullptr, newId, vsString, fsString , false, false });
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

RARetCode GraphicsManager::removeShader(SHADER_ID id)
{
	for (auto& shaderHolder : mShaderHolders) {
		if (shaderHolder.mId == id) {
			shaderHolder.mRemove = true;
			return RARetCode::RET_OK;
		}
	}
	return RARetCode::RET_ERR_INVALID_ARG;
}


RARetCode GraphicsManager::setFont(std::string file, FONT_ID& id)
{
	FONT_ID newId = static_cast<FONT_ID>(mFontHolders.size() + 1);
	mFontHolders.emplace_back(FontHolder{ nullptr, newId, file, false });
	id = newId;
	return RARetCode::RET_OK;
}

void* GraphicsManager::getFont(FONT_ID id)
{
	for (auto& fontHolder : mFontHolders) {
		if (fontHolder.mId == id) {
			if (fontHolder.mFont) {
				return fontHolder.mFont;
			}
			else {
				Utility::printLog("Font not readly");
				return nullptr;
			}
		}
	}
	return nullptr;

}

RARetCode GraphicsManager::removeFont(FONT_ID id)
{
	for (auto& fontHolder : mFontHolders) {
		if (fontHolder.mId == id) {
			fontHolder.mRemove = true;
			return RARetCode::RET_OK;
		}
	}
	return RARetCode::RET_ERR_INVALID_ARG;
}


RARetCode GraphicsManager::setModel(std::string file, MODEL_ID& id)
{
	MODEL_ID newId = static_cast<MODEL_ID>(mModelHolders.size() + 1);
	mModelHolders.emplace_back(ModelHolder{ nullptr, newId, file, false });
	id = newId;
	return RARetCode::RET_OK;
}

void* GraphicsManager::getModel(MODEL_ID id)
{
	for (auto& modelHolder : mModelHolders) {
		if (modelHolder.mId == id) {
			if (modelHolder.mModel) {
				return modelHolder.mModel;
			}
			else {
				Utility::printLog("Model not readly");
				return nullptr;
			}
		}
	}
	return nullptr;
}

RARetCode GraphicsManager::removeModel(MODEL_ID id)
{
	for (auto& modelHolder : mModelHolders) {
		if (modelHolder.mId == id) {
			modelHolder.mRemove = true;
			return RARetCode::RET_OK;
		}
	}
	return RARetCode::RET_ERR_INVALID_ARG;
}


////////////////////////////////////
// Private
////////////////////////////////////

