#pragma once

#include <memory>

class ObjectRepository {
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
	static void createInstance();
	static void destroyInstance();
	static ObjectRepository& getInstance();

	void	registerObject(Object* obj);
	void    removeObject(Object* obj);
	Object* getObject(unsigned int id);

	virtual ~ObjectRepository() = default;
private:
	static std::unique_ptr<ObjectRepository> mInstance;

	ObjectRepository() = default;


};