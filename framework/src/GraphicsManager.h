#pragma once

#include <mutex>
#include <vector>

#include "IGraphicsManager.h"

class GraphicsManager : public IGraphicsManager
{
public:
	void runUntilClosed();

	void setRenderer(IObjectRenderer* renderer);
	void removeRenderer(IObjectRenderer* renderer);

	GraphicsManager();
	virtual ~GraphicsManager() = default;

private:
	std::mutex		   mMutex;

	std::vector<IObjectRenderer*>
		mRenderers;
};

