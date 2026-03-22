#pragma once

class IWorldNavigator {
public:
	typedef struct {
		int mX;
		int mY;
		int mZ;
	} Vec3;

	typedef struct {
		Vec3 mMin;
		Vec3 mMax;
		Vec3 mNonScrollRange;
		unsigned int mRate;
	} SpaceConfig;

	typedef struct {
		Vec3 mMin;
		Vec3 mMax;
	} ActiveSpace;

	typedef unsigned int WORLD_ID;

	#define isValidWorldId(x) 0!=x 

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	virtual WORLD_ID	 createWorld(SpaceConfig& space)    = 0;
	virtual WORLD_ID	 getCurrentWorld()				    = 0;
	virtual void		 setCurrentWorld(WORLD_ID id)       = 0;
	virtual WORLD_ID	 changeWorld(WORLD_ID id)	        = 0;
	virtual ActiveSpace& getActiveSpace()					= 0;
	virtual void		 setActiveSpace(ActiveSpace& space) = 0;
};