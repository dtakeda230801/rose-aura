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
	};

	struct FontHolder {
		void*		mFont;
		FONT_ID		mId;
		std::string mFontFile;
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
	RARetCode setFont(std::string file, FONT_ID& id);
	void*     getFont(FONT_ID id);



	GraphicsManager() = default;
	virtual ~GraphicsManager() = default;

private:
	std::mutex						mMutex;
	std::vector<IGraphicsRenderer*>	mRenderers;
	std::vector<ShaderHolder>	    mShaderHolders;
	std::vector<FontHolder>			mFontHolders;
	std::atomic<bool>				mRunning;
};

