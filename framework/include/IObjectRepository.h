#pragma once

#include <vector>
#include <memory>
#include <functional>

#include "RoseAuraReturnCode.h"

using namespace RoseAuraReturnCode;

class IObjectRepository {
public:

	using OBJECT_ID = unsigned int;
	using TAG_ID    = unsigned int;

#define isValidObjectId(x) (0 < x)

    struct ObjectBinder
    {
        std::function<void* (void*)>create;
        std::function<void (void*)>destroy;
        void*  params;
        std::function<void (void*)>destroyParams;
    };

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
    template<class T, class... Args>
    std::unique_ptr<ObjectBinder> makeObjectBinder(void (T::*initializer)(), void (T::*terminator)(), Args&&... args)
    {
        using Tuple  = std::tuple<Args...>;
        Tuple* tuple = new Tuple(std::forward<Args>(args)...);
        std::unique_ptr<ObjectBinder> ret = std::make_unique<ObjectBinder>();

        // === create ===
        ret->create = [initializer](void* data)->void* {
            auto tuple = static_cast<Tuple*>(data);

            T* obj = std::apply(
                  [](auto&&... xs) { return new T(std::forward<decltype(xs)>(xs)...); }
                , *tuple);

            if (initializer) {
                (obj->*initializer)();
            }

            return obj;
        };

        // === destroy ===
        ret->destroy = [terminator](void* p) {
            T* obj = static_cast<T*>(p);

            if (terminator) {
                (obj->*terminator)();
            }

            delete obj;
        };

        // === params ===
        ret->params = tuple;

        // === destroy params ===
        ret->destroyParams = [](void* p) {
            delete static_cast<Tuple*>(p);
        };

        return ret;
    }

	virtual OBJECT_ID registerObject(std::unique_ptr<ObjectBinder> binder)                            = 0;
    virtual OBJECT_ID registerObject(std::unique_ptr<ObjectBinder> binder, std::vector<TAG_ID>& tags) = 0;

    virtual RARetCode unregisterObject(OBJECT_ID id)       = 0;
	
    virtual RARetCode addTag(OBJECT_ID id, TAG_ID tag)     = 0;
	virtual RARetCode removeTag(OBJECT_ID id, TAG_ID tag)  = 0;

    virtual RARetCode activate(OBJECT_ID id)   = 0;
    virtual RARetCode deactivate(OBJECT_ID id) = 0;

    virtual RARetCode activateByTag(TAG_ID id)   = 0;
    virtual RARetCode deactivateByTag(TAG_ID id) = 0;

    virtual bool isActivate(OBJECT_ID id)   = 0;
    virtual bool isActivateByTag(TAG_ID id) = 0;

};