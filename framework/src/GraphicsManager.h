#pragma once

#include <mutex>
#include <vector>

#include "IGraphicsManager.h"

class GraphicsManager : public IGraphicsManager
{
public:
	void	  runUntilClosed();
	RARetCode setRenderer(IObjectRenderer* renderer);
	RARetCode removeRenderer(IObjectRenderer* renderer);

	GraphicsManager() = default;
	virtual ~GraphicsManager() = default;

private:
	std::mutex		   mMutex;

	std::vector<IObjectRenderer*>
		mRenderers;
};

