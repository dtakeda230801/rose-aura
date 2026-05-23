#include "raylib.h"

#include "GraphicsManager.h"
#include "Utility.h"


////////////////////////////////////
// APIs
////////////////////////////////////
struct GraphicsManager::ModelWrapper::ModelHolder {
	Model			mModel;
	ModelAnimation* mModelAnimation;
};

IGraphicsManager::MODEL_ID  GraphicsManager::ModelWrapper::getModelId()
{
	return mId;
}


void* GraphicsManager::ModelWrapper::getModel()
{
	return static_cast<void*>(&mModelHolder->mModel);
}

void* GraphicsManager::ModelWrapper::getAnimetionModel()
{
	if (!mModelHolder->mModelAnimation) {
		return nullptr;
	}

	int frameLen = mModelHolder->mModelAnimation[mCurrentAnimation].keyframeCount;

	mFrame += 1.0f/mRate;

	if (mFrame >= static_cast<float>(frameLen - mEndOffset)) {
		mFrame = 1.0f;
	}

	if (mFrame < static_cast<float>(mStartOffset)) {
		mFrame = static_cast<float>(mStartOffset);
	}


	UpdateModelAnimation(mModelHolder->mModel, mModelHolder->mModelAnimation[mCurrentAnimation], mFrame);

	return static_cast<void*>(&mModelHolder->mModel);
}

uint32_t  GraphicsManager::ModelWrapper::getAnimationNum()
{
	return mAnimationNum;
}

RARetCode GraphicsManager::ModelWrapper::selectAnimation(uint32_t index)
{
	if (!mModelHolder->mModelAnimation) {
		return RARetCode::RET_ERR_INVALID_STATE;
	}

	if (index >= static_cast<uint32_t>(mAnimationNum)) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}

	if (mCurrentAnimation != index) {

		mCurrentAnimation = index;
		mFrame			  = 0.0f;
	}

	return RARetCode::RET_OK;
}

void GraphicsManager::ModelWrapper::setAdjustment(uint32_t startOffset, uint32_t endOffset, float rate)
{
	mStartOffset = startOffset;
	mEndOffset   = endOffset;
	mRate        = rate;
}


void GraphicsManager::ModelWrapper::load()
{
	if (!mIsLoaded) {
		mModelHolder->mModel = LoadModel(mModelFile.c_str());

		if (mLoadAnimation) {
			mModelHolder->mModelAnimation = LoadModelAnimations(mModelFile.c_str(), &mAnimationNum);

			for (int i = 0; i < mAnimationNum; ++i) {
				Utility::printLog("Animation : %s / %d", mModelHolder->mModelAnimation[i].name
					                                   , mModelHolder->mModelAnimation[i].keyframeCount);
			}
		}
		mIsLoaded = true;
	}
}

bool GraphicsManager::ModelWrapper::isLoaded() {
	return mIsLoaded;
}

void GraphicsManager::ModelWrapper::setUnloadResevation()
{
	mReleaseResevation = true;
}

bool GraphicsManager::ModelWrapper::hasUnloadResevation()
{
	return mReleaseResevation;
}

void GraphicsManager::ModelWrapper::unload()
{
	if (mIsLoaded) {
		if (mModelHolder->mModelAnimation) {
			UnloadModelAnimations(mModelHolder->mModelAnimation, mAnimationNum);
			mModelHolder->mModelAnimation = nullptr;
			mAnimationNum = 0;
		}

		UnloadModel(mModelHolder->mModel);
		if (mModelHolder->mModel.currentPose) {
			free(mModelHolder->mModel.currentPose);
			mModelHolder->mModel.currentPose = nullptr;
		}
		if (mModelHolder->mModel.boneMatrices) {
			free(mModelHolder->mModel.boneMatrices);
			mModelHolder->mModel.boneMatrices = nullptr;
		}


		mIsLoaded = false;
	}
}

GraphicsManager::ModelWrapper::ModelWrapper(MODEL_ID id, std::string model, bool loadAnimation)
	: mId(id)
	, mModelFile(model)
	, mLoadAnimation(loadAnimation)
	, mModelHolder(std::make_unique<ModelHolder>())
	, mIsLoaded(false)
	, mAnimationNum(0)
	, mStartOffset(0)
	, mEndOffset(0)
	, mRate(1.0f)
	, mFrame(0.0f)
	, mCurrentAnimation(0)
	, mReleaseResevation(false)
{
}

GraphicsManager::ModelWrapper::~ModelWrapper()
{
}


////////////////////////////////////
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

		for (auto& wrapper : mModelWrappers) {
			if (wrapper && !wrapper->isLoaded()) {
				wrapper->load();
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

		for (auto& wrapper : mModelWrappers) {
			if (wrapper && wrapper->hasUnloadResevation()) {
				wrapper->unload();
				wrapper.reset();
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

	for (auto& wrapper : mModelWrappers) {
		if (wrapper) {
			wrapper->unload();
			wrapper.reset();
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

IGraphicsManager::SHADER_ID GraphicsManager::setShaderFile(std::string vsFile, std::string fsFile)
{	
	SHADER_ID newId = static_cast<SHADER_ID>(mShaderHolders.size() + 1);
	mShaderHolders.emplace_back(ShaderHolder{ nullptr, newId, vsFile, fsFile , true });
	return newId;
}

IGraphicsManager::SHADER_ID GraphicsManager::setShader(std::string vsStr, std::string fsStr)
{
	SHADER_ID newId = static_cast<SHADER_ID>(mShaderHolders.size() + 1);
	mShaderHolders.emplace_back(ShaderHolder{ nullptr, newId, vsStr, fsStr , false, false });
	return newId;
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


IGraphicsManager::FONT_ID GraphicsManager::setFont(std::string file)
{
	FONT_ID newId = static_cast<FONT_ID>(mFontHolders.size() + 1);
	mFontHolders.emplace_back(FontHolder{ nullptr, newId, file, false });

	if (mFontHolders.size() == 1) {
		mDefaultFont = newId;
	}

	return newId;
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

RARetCode GraphicsManager::setDefaultFont(FONT_ID id)
{
	if (mFontHolders.size() == 0) {
		return RARetCode::RET_ERR_INVALID_STATE;
	}

	for (auto& fontHolder : mFontHolders) {
		if (fontHolder.mId == id) {
			mDefaultFont = id;
			return RARetCode::RET_OK;
		}
	}
	return RARetCode::RET_ERR_INVALID_ARG;

}

void* GraphicsManager::getDefaultFont()
{
	if (mFontHolders.size() == 0) {
		return nullptr;
	}

	return getFont(mDefaultFont);
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

IGraphicsManager::MODEL_ID GraphicsManager::setModel(std::string file, bool loadAnimation)
{
	MODEL_ID newId = static_cast<uint32_t>(mModelWrappers.size() + 1);
	mModelWrappers.push_back(std::make_unique<ModelWrapper>(newId, file, loadAnimation));
	return newId;
}


IGraphicsManager::IModelWrapper* GraphicsManager::getModelWrapper(IGraphicsManager::MODEL_ID id)
{
	for (auto& wrapper : mModelWrappers) {
		if (wrapper && wrapper->getModelId() == id) {
			return wrapper.get();
		}
	}
	return nullptr;
}


void GraphicsManager::releaseModelWrapper(MODEL_ID id)
{
	for (auto& wrapper : mModelWrappers) {
		if (wrapper && wrapper->getModelId() == id) {
			wrapper->setUnloadResevation();
		}
	}
}

////////////////////////////////////
// Private
////////////////////////////////////

