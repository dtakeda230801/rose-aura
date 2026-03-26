#pragma once

class IWorldNavigator {
public:
	struct Vec3 {
		int mX;
		int mY;
		int mZ;
	};

	struct ActiveSpace {
		Vec3 mMin;
		Vec3 mMax;
	};

	typedef struct {
		Vec3		mMin;
		Vec3		mMax;
		Vec3		mNonScrollRange;
		ActiveSpace mActiveSpace;
	} WorldConfig;

	using WORLD_ID = unsigned int;
	using TRIGGER_ID = unsigned int;

	class IApproachingCallback {
	public:
		virtual bool onApproaching(WORLD_ID worldId, TRIGGER_ID eventId, Vec3& position) = 0;

		virtual ~IApproachingCallback() = default;
	protected:
		IApproachingCallback() = default;
	};

	class ITriggerCallback {
	public:
		virtual void onTrigger(WORLD_ID worldId, TRIGGER_ID eventId) = 0;

		virtual ~ITriggerCallback() = default;
	protected:
		ITriggerCallback() = default;
	};

	class IActiveSpaceCallback {
	public:
		virtual void onUpdate(WORLD_ID worldId, ActiveSpace activeSpace) = 0;

		virtual ~IActiveSpaceCallback() = default;
	protected:
		IActiveSpaceCallback() = default;
	};


	#define isValidWorldId(x) 0!=x 

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	virtual WORLD_ID	 createWorld(WorldConfig& space)    = 0;
	virtual WORLD_ID	 getCurrentWorld()				    = 0;
	virtual WORLD_ID	 changeWorld(WORLD_ID id)	        = 0;
	virtual ActiveSpace& getActiveSpace()					= 0;
	virtual int 		 setActiveSpace(ActiveSpace& space) = 0;
	virtual int          updatePosition(Vec3& center)		= 0;
	virtual Vec3&		 getPosition()						= 0;

	virtual int			 registerTrigger(TRIGGER_ID trigger, Vec3& location, float distance) = 0;
	virtual int			 removeTrigger(TRIGGER_ID trigger)	= 0;

	virtual void		 registerApproachingCallback(IApproachingCallback* approachingCallback) = 0;
	virtual void		 unregisterApproachingCallback(IApproachingCallback* approachingCallback) = 0;
	virtual void		 registerTriggerCallback(ITriggerCallback* triggerCallback) = 0;
	virtual void		 unregisterTriggerCallback(ITriggerCallback* triggerCallback) = 0;
	virtual void		 registerActiveSpaceUpdate(IActiveSpaceCallback* activeSpaceCallback) = 0;
	virtual void		 unregisterActiveSpaceUpdate(IActiveSpaceCallback* activeSpaceCallback) = 0;
};

