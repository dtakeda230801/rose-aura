#pragma once

#include <mutex>
#include <vector>
#include <string>

#include "IGraphicsManager.h"

class GraphicsManager : public IGraphicsManager
{
public:
	struct ShaderHolder {
		void*		mShader;
		SHADER_ID	mId;
		std::string mVShader;
		std::string mFShader;
		bool		mIsFile;
		bool        mRemove;
	};

	struct FontHolder {
		void*		mFont;
		FONT_ID		mId;
		std::string mFontFile;
		bool        mRemove;
	};

	class ModelWrapper : public IModelWrapper
	{
	public:
		MODEL_ID  getModelId();
		void*     getModel();
		void*     getAnimetionModel(bool forward);
		uint32_t  getAnimationNum();
		RARetCode selectAndResetAnimation(uint32_t index);
		bool      isMoving();
		void      setAdjustment(uint32_t start
			                  , uint32_t end
			                  , float    rate
			                  , std::vector<uint32_t> stableFrames);

		void      load();
		bool      isLoaded();

		void	  setUnloadResevation();
		bool      hasUnloadResevation();
		void	  unload();

		ModelWrapper(MODEL_ID id, std::string model, bool loadAnimation);
		virtual ~ModelWrapper();

	private:
		MODEL_ID        mId;
		std::string     mModelFile;
		bool            mLoadAnimation;
		bool            mIsLoaded;
		int32_t         mAnimationNum;
		float	        mFrame;
		uint32_t        mCurrentAnimation;
		bool		    mReleaseResevation;
		uint32_t		mStartOffset;
		uint32_t		mEndOffset;
		float    		mRate;
		bool            mIsMoving;
		std::vector<uint32_t> 
			            mSstableFrames;

		struct			ModelHolder;
		std::unique_ptr<ModelHolder>    mModelHolder;
	};

	void	  runUntilClosed(Conf conf);
	void      exit();

	RARetCode setRenderer(IGraphicsRenderer* renderer, Layer layer);
	RARetCode removeRenderer(IGraphicsRenderer* renderer);

	RARetCode updateCamera(void* camera);

	SHADER_ID setShaderFile(std::string vsFile
		                  , std::string fsFile);
	SHADER_ID setShader(std::string     vsStr
		              , std::string     fsStr);
	void*     getShader(SHADER_ID id);
	RARetCode removeShader(SHADER_ID id);

	FONT_ID   setFont(std::string file);
	void*     getFont(FONT_ID id);
	RARetCode setDefaultFont(FONT_ID id);
	void*     getDefaultFont();
	RARetCode removeFont(FONT_ID id);

	MODEL_ID       setModel(std::string file, bool loadAnimation);
	IModelWrapper* getModelWrapper(MODEL_ID id);
	void           releaseModelWrapper(MODEL_ID id);

	GraphicsManager();
	virtual ~GraphicsManager();

private:
	struct RendererHolder {
		IGraphicsRenderer* mRenderer;
		Layer              mLayer;
	};


	std::mutex									mMutex;
	std::vector<RendererHolder>					mRenderers;
	std::vector<ShaderHolder>					mShaderHolders;
	std::vector<FontHolder>						mFontHolders;
	std::vector<std::unique_ptr<ModelWrapper>> 	mModelWrappers;
	std::atomic<bool>							mRunning;

	void*		mCamera;
	FONT_ID		mDefaultFont;
};

