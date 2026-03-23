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
	} SpaceConfig;

	using WORLD_ID = unsigned int;
	using EVENT_ID = unsigned int;

	class IApproachingCallback {
	public:
		virtual bool checkApproaching(WORLD_ID worldId, EVENT_ID eventId, Vec3& position) = 0;

		IApproachingCallback() = default;
		virtual ~IApproachingCallback() = default;
	};

	class IEventCallback {
	public:
		virtual void onEvent(WORLD_ID worldId, EVENT_ID eventId) = 0;

		IEventCallback() = default;
		virtual ~IEventCallback() = default;
	};


	#define isValidWorldId(x) 0!=x 

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	virtual WORLD_ID	 createWorld(SpaceConfig& space)    = 0;
	virtual WORLD_ID	 getCurrentWorld()				    = 0;
	virtual int 		 setCurrentWorld(WORLD_ID id)       = 0;
	virtual WORLD_ID	 changeWorld(WORLD_ID id)	        = 0;
	virtual ActiveSpace& getActiveSpace()					= 0;
	virtual int 		 setActiveSpace(ActiveSpace& space) = 0;
	virtual int          updatePosition(Vec3& center)		= 0;
	virtual Vec3&		 getPosition()						= 0;

	virtual int			 registerEvent(EVENT_ID ev, Vec3& location, float distance) = 0;
	virtual int			 removeEvent(EVENT_ID ev)	= 0;

	virtual void		 registerApproachingCallback(IApproachingCallback* approachingCallback) = 0;
	virtual void		 registerEventCallback() = 0;
};
