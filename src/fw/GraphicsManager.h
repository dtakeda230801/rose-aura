#pragma once

class GraphicsManager
{
public:
	static GraphicsManager* getInstance();
	static void destroyInstance();

	void run();

private:
	GraphicsManager();
	virtual ~GraphicsManager() = default;

	static GraphicsManager* sInstance;
};

