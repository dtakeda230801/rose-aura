#pragma once

#include "RoseAuraReturnCode.h"

#define isValidWorldId(x) (0 != x) 

class IWorldNavigator {
public:
	using WORLD_ID   = uint32_t;
	using ENTITY_ID  = uint32_t;

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

	class ICollisionCallback {
	public:

		enum class CollisionResult {
              NO_COLLISION,
			  HIT,
			  INHIBITED
		};

		virtual CollisionResult onApproaching(WORLD_ID worldId, ENTITY_ID entityId, Vec3 from, Vec3& to) = 0;
		virtual void onHit(WORLD_ID worldId, ENTITY_ID entityId) = 0;

		virtual ~ICollisionCallback() = default;
	protected:
		ICollisionCallback() = default;
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
		Vec3        mPrimaryEntityPosition;
		bool        mEnableFollowing;
		bool		mLimitScrolling;
	};

	static constexpr ENTITY_ID	PRIMARY_ENTITY_ID = 0;

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

	virtual RARetCode movePrimaryEntityPosition(Vec3& pos) = 0;
	virtual Vec3	  getPrimaryEntityPosition()		   = 0;

	virtual RARetCode registerCollisionCallback(ICollisionCallback* cb) = 0;
	virtual RARetCode unregisterCollisionCallback()                     = 0;

	virtual RARetCode registerEntity(ENTITY_ID			id
		                            , Vec3&				location
		                            , float				distance
		                            , ICollisionCallback* cb)			= 0;
	virtual RARetCode removeEntity(ENTITY_ID id)					    = 0;
	virtual RARetCode moveEntity(ENTITY_ID id, Vec3& location)	        = 0;
	virtual RARetCode getEntityLocation(ENTITY_ID	id, Vec3* location)	= 0;

	virtual RARetCode registerActiveSpaceCallback(IActiveSpaceCallback* cb) = 0;
	virtual RARetCode unregisterActiveSpaceCallback()						= 0;
};

