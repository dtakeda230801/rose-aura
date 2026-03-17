#pragma once

#include "IObjectRepository.h"

class ObjectRepository : public IObjectRepository {
public:
	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	void	registerObject(Object* obj);
	void    removeObject(Object* obj);
	Object* getObject(unsigned int id);

	ObjectRepository() = default;
	virtual ~ObjectRepository() = default;
};