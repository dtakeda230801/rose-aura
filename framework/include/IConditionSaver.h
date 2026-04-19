#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <any>
#include <typeindex>

#include "RoseAuraReturnCode.h"

using namespace RoseAuraReturnCode;

class IConditionSaver {
public:

	using CONDITION_SET_ID = uint32_t;

	class ISerializer {
	public:
		virtual RARetCode serialize(std::any data) = 0;
	};

	class IDeserializer {
	public:
		virtual RARetCode deserialize(std::type_index type, std::any& out) = 0;
	};

	class IConditionSet {
	public:
		virtual CONDITION_SET_ID getConditionId()                                       = 0;
		virtual void             setConditionSetName(std::string condSetName)           = 0;
		virtual RARetCode        setCondition(std::string condName, std::string value)  = 0;
		virtual RARetCode        getCondition(std::string condName, std::string& value) = 0;

		virtual ISerializer&     getSerializer(std::string condName)					= 0;
		virtual IDeserializer&   getDeserializer(std::string condName)				    = 0;

		virtual RARetCode        saveToFile(const char* filename)						= 0;
		virtual RARetCode        loadFromFile(const char* filename)						= 0;
	};

	virtual IConditionSet&		 createConditionSet()                            = 0;
	virtual RARetCode			 destroyConditionSet(CONDITION_SET_ID condSetId) = 0;
};
