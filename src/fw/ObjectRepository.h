#pragma once

#include <memory>

class ObjectRepository {
public:
	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	static void createInstance();
	static void destroyInstance();
	static ObjectRepository& getInstance();

	virtual ~ObjectRepository() = default;
private:
	static std::unique_ptr<ObjectRepository> mInstance;

	ObjectRepository() = default;


};