#pragma once

#include <vector>

class IObjectRepository {
public:

	using OBJECT_ID = unsigned int;
	using GROUP_ID  = unsigned int;

	class Object {
	public:
		virtual OBJECT_ID getObjectId() = 0;

		virtual ~Object() = default;
	protected:
		Object() = default;
	};

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	virtual void	registerObject(Object* obj)    = 0;
	virtual void    removeObject(Object* obj)      = 0;
	virtual Object& getObject(OBJECT_ID id)        = 0;
	virtual std::vector<Object&>& 
		            getObjectsByGroup(GROUP_ID id) = 0;
};