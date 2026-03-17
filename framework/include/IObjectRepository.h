#pragma once

class IObjectRepository {
public:
	class Object {
	public:
		virtual unsigned int getObjectId() = 0;

		virtual ~Object() = default;
	private:
		Object() = default;
	};

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	virtual void	registerObject(Object* obj) = 0;
	virtual void    removeObject(Object* obj)   = 0;
	virtual Object* getObject(unsigned int id)  = 0;
};