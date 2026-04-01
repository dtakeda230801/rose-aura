#pragma once

#include "RoseAuraReturnCode.h"

using namespace RoseAuraReturnCode;

class IWorldNavigator {
public:
	using WORLD_ID = unsigned int;
	using TRIGGER_ID = unsigned int;

#define isValidWorldId(x) (0 != x) 

	struct Vec3 {
		int mX;
		int mY;
		int mZ;
	};

	struct Extent {
		unsigned int mX;
		unsigned int mY;
		unsigned int mZ;
	};

	struct Bounds {
		Vec3 mMin;
		Vec3 mMax;
	};

	class ITriggerCallback {
	public:
		virtual bool onApproaching(WORLD_ID worldId, TRIGGER_ID eventId, Vec3& trigerLocation, Vec3& position) = 0;
		virtual void onTrigger(WORLD_ID worldId, TRIGGER_ID eventId) = 0;

		virtual ~ITriggerCallback() = default;
	protected:
		ITriggerCallback() = default;
	};

	class IActiveSpaceCallback {
	public:
		virtual void onUpdate(WORLD_ID worldId, Bounds activeSpace) = 0;

		virtual ~IActiveSpaceCallback() = default;
	protected:
		IActiveSpaceCallback() = default;
	};

	struct WorldConfig {
		Bounds      mWorldSpace;
		Extent      mActiveRange;
		Extent		mNonScrollRange;
		Vec3        mPosition;
		bool        mEnableFollowing;
		bool		mLimitScrolling;
	};

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	virtual WORLD_ID  createWorld(WorldConfig& space)    = 0;
	virtual WORLD_ID  getCurrentWorld()			         = 0;
	virtual RARetCode changeWorld(WORLD_ID id)	         = 0;
	virtual Bounds	  getActiveSpace()					 = 0;
	virtual Vec3      getActiveSpacePosition()			 = 0;
	virtual RARetCode moveActiveSpace(Vec3& pos)		 = 0;
	virtual void	  resetActiveSpace()				 = 0;
	virtual RARetCode movePosition(Vec3& pos)			 = 0;
	virtual Vec3	  getPosition()						 = 0;

	virtual RARetCode registerTrigger(TRIGGER_ID id, Vec3& location, float distance, ITriggerCallback* cb) = 0;
	virtual RARetCode removeTrigger(TRIGGER_ID id)	= 0;

	virtual RARetCode registerActiveSpaceCallback(IActiveSpaceCallback* cb)   = 0;
	virtual RARetCode unregisterActiveSpaceCallback() = 0;
};

