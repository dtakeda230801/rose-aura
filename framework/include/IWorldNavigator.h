#pragma once

class IWorldNavigator {
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
	virtual unsigned int createWorld(SpaceConfig& space) = 0;
	virtual unsigned int getCurrentWorld()				 = 0;
	virtual int			 changeWorld(unsigned int id)	 = 0;
};