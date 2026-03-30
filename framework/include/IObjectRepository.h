#pragma once

#include <vector>
#include <memory>

#include "RoseAuraReturnCode.h"

using namespace RoseAuraReturnCode;

class IObjectRepository {
public:

	using OBJECT_ID = unsigned int;
	using TAG_ID = unsigned int;

#define isValidObjectId(x) (0 < x)

    struct ObjectBinder
    {
        void* (*create)(void*);
        void  (*destroy)(void*);
        void  (*destroyParams)(void*);
        void*  params;
    };

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
    template<class T, class... Args>
    ObjectBinder makeObjectBinder(Args&&... args)
    {
        using Tuple = std::tuple<std::decay_t<Args>...>;

        Tuple* tuple =
            new Tuple(std::forward<Args>(args)...);

        return {
            // create
            [](void* data)->void*
            {
                auto params =
                    static_cast<Tuple*>(data);

                return std::apply(
                    [](auto&&... xs)
                    {
                        return static_cast<void*>(
                            new T(std::forward<decltype(xs)>(xs)...));
                    },
                    *params);
            },
            // destroy
            [](void* p)
            {
                delete static_cast<T*>(p);
            },

            // destroy params
            [](void* p)
            {
                delete static_cast<Tuple*>(p);
            },

            // params
            tuple
        };
    }

	virtual OBJECT_ID registerObject(std::unique_ptr<ObjectBinder> binder) = 0;
	virtual RARetCode unregisterObject(OBJECT_ID id)                       = 0;
	virtual RARetCode addTag(OBJECT_ID id, std::vector<TAG_ID>& tags)      = 0;
	virtual RARetCode removeTag(OBJECT_ID id, TAG_ID tag)                  = 0;

    virtual RARetCode activate(OBJECT_ID id)   = 0;
    virtual RARetCode deactivate(OBJECT_ID id) = 0;

    virtual RARetCode activateByTag(TAG_ID id)   = 0;
    virtual RARetCode deactivateByTag(TAG_ID id) = 0;
};