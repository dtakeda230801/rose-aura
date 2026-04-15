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
		std::string mFileName;
	};

	void	  runUntilClosed(Conf conf);
	RARetCode setRenderer(IGraphicsRenderer* renderer);
	RARetCode removeRenderer(IGraphicsRenderer* renderer);
	RARetCode setShaderFile(std::string file);
	void*     getShader(std::string file);

	GraphicsManager() = default;
	virtual ~GraphicsManager() = default;

private:
	std::mutex						mMutex;
	std::vector<IGraphicsRenderer*>	mRenderers;
	std::vector<ShaderHolder>	    mShaderHolders;
};

