#pragma once

#include <memory>
#include <mutex>
#include <vector>


class GraphicsManager
{
public:
	class IObjectRenderer {
	public:
		virtual void render() = 0;

		virtual ~IObjectRenderer() = default;
	protected:
		IObjectRenderer() = default;
	};

	static void createInstance();
	static void destroyInstance();
	static GraphicsManager& getInstance();

	void runUntilClosed();

	void setRenderer(IObjectRenderer* renderer);
	void removeRenderer(IObjectRenderer* renderer);

	virtual ~GraphicsManager() = default;
private:
	GraphicsManager();

	static std::unique_ptr<GraphicsManager> mInstance;

	std::mutex		   mMutex;

	std::vector<IObjectRenderer*>
		mRenderers;
};

