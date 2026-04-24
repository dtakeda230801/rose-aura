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

	void	  runUntilClosed(Conf conf);
	RARetCode setRenderer(IGraphicsRenderer* renderer);
	RARetCode removeRenderer(IGraphicsRenderer* renderer);
	RARetCode setShaderFile(std::string vsfile
					      , std::string fsfile
						  , SHADER_ID& id);
	RARetCode setShader(std::string vsString
					  , std::string fsString
					  , SHADER_ID& id);

	void*     getShader(SHADER_ID id);

	GraphicsManager() = default;
	virtual ~GraphicsManager() = default;

private:
	std::mutex						mMutex;
	std::vector<IGraphicsRenderer*>	mRenderers;
	std::vector<ShaderHolder>	    mShaderHolders;
};

