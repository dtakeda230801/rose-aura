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
		void*     getAnimetionModel();
		uint32_t  getAnimationNum();
		RARetCode selectAnimation(uint32_t index);
		void      setAdjustment(uint32_t start
			                  , uint32_t end
			                  , float    rate);

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

		struct			ModelHolder;
		std::unique_ptr<ModelHolder>    mModelHolder;
	};

	void	  runUntilClosed(Conf conf);
	void      exit();

	RARetCode setRenderer(IGraphicsRenderer* renderer);
	RARetCode removeRenderer(IGraphicsRenderer* renderer);

	SHADER_ID setShaderFile(std::string vsFile
		                  , std::string fsFile);
	SHADER_ID setShader(std::string     vsStr
		              , std::string     fsStr);
	void*     getShader(SHADER_ID id);
	RARetCode removeShader(SHADER_ID id);

	FONT_ID   setFont(std::string file);
	void*     getFont(FONT_ID id);
	RARetCode removeFont(FONT_ID id);

	MODEL_ID       setModel(std::string file, bool loadAnimation);
	IModelWrapper* getModelWrapper(MODEL_ID id);
	void           releaseModelWrapper(MODEL_ID id);

	GraphicsManager() = default;
	virtual ~GraphicsManager() = default;

private:
	std::mutex									mMutex;
	std::vector<IGraphicsRenderer*>				mRenderers;
	std::vector<ShaderHolder>					mShaderHolders;
	std::vector<FontHolder>						mFontHolders;
	std::vector<std::unique_ptr<ModelWrapper>> 	mModelWrappers;
	std::atomic<bool>							mRunning;
};

