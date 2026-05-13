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

	struct ModelHolder {
		void*       mModel;
		MODEL_ID	mId;
		std::string mModelFile;
		bool        mRemove;
	};


	void	  runUntilClosed(Conf conf);
	void      exit();

	RARetCode setRenderer(IGraphicsRenderer* renderer);
	RARetCode removeRenderer(IGraphicsRenderer* renderer);

	RARetCode setShaderFile(std::string vsfile
					      , std::string fsfile
						  , SHADER_ID& id);
	RARetCode setShader(std::string vsString
					  , std::string fsString
					  , SHADER_ID& id);
	void*     getShader(SHADER_ID id);
	RARetCode removeShader(SHADER_ID id);

	RARetCode setFont(std::string file, FONT_ID& id);
	void*     getFont(FONT_ID id);
	RARetCode removeFont(FONT_ID id);


	RARetCode setModel(std::string file, MODEL_ID& id);
	void*     getModel(MODEL_ID id);
	RARetCode removeModel(MODEL_ID id);

	GraphicsManager() = default;
	virtual ~GraphicsManager() = default;

private:
	std::mutex						mMutex;
	std::vector<IGraphicsRenderer*>	mRenderers;
	std::vector<ShaderHolder>	    mShaderHolders;
	std::vector<FontHolder>			mFontHolders;
	std::vector<ModelHolder>		mModelHolders;
	std::atomic<bool>				mRunning;
};

