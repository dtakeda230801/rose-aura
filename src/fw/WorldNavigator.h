#pragma once

#include <memory>

class WorldNavigator {
public:
	typedef struct {
		unsigned int mXMin;
		unsigned int mYMin;
		unsigned int mZMin;
		unsigned int mXMax;
		unsigned int mYMax;
		unsigned int mZMax;
		unsigned int mRate;
	} SpaceConfig;

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	static void createInstance();
	static void destroyInstance();
	static WorldNavigator& getInstance();

	unsigned int createWorld(SpaceConfig& space);
	unsigned int getCurrentWorld();
	int			 changeWorld(unsigned int id);

	virtual ~WorldNavigator() = default;
private:
	static std::unique_ptr<WorldNavigator> mInstance;

	WorldNavigator() = default;


};