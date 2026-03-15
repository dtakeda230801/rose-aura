#pragma once

#include <memory>

class WorldNavigator {
public:
	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	static void createInstance();
	static void destroyInstance();
	static WorldNavigator& getInstance();

	virtual ~WorldNavigator() = default;
private:
	static std::unique_ptr<WorldNavigator> mInstance;

	WorldNavigator() = default;


};