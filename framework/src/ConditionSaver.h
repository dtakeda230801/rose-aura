#pragma once

#include <vector>
#include <memory>

#include "IConditionSaver.h"

class ConditionSaver : public IConditionSaver
{
public:

	class ConditionSet;

	class Serializer : public ISerializer
	{
	public:
		RARetCode serialize(std::any data);

		Serializer(std::string& container);
		virtual ~Serializer() = default;
	private:
		std::string&	mContainer;

	};

	class Deserializer : public IDeserializer
	{
	public:
		RARetCode deserialize(std::type_index type, std::any& out);

		Deserializer(std::string& container);
		virtual ~Deserializer() = default;
	private:
		std::string&	mContainer;
		uint32_t		mPointer;
	};

	class ConditionSet : public IConditionSet
	{
	public:
		CONDITION_SET_ID getConditionId();
		void             setConditionSetName(std::string condSetName);
		RARetCode        setCondition(std::string condName, std::string value);
		RARetCode        getCondition(std::string condName, std::string& value);

		ISerializer&     getSerializer(std::string condName);
		IDeserializer&   getDeserializer(std::string condName);

		RARetCode        saveToFile(const char* filename);
		RARetCode        loadFromFile(const char* filename);

		ConditionSet(CONDITION_SET_ID id);
		virtual ~ConditionSet();

	private:
		CONDITION_SET_ID	mId;
		std::string         mCondSetName;

		std::vector<std::pair<std::string, std::string>>
							mRegularConds;
		std::vector<std::pair<std::string, std::unique_ptr<std::string>>>
							mSerializedConds;

		std::vector<Serializer*>	mSerializers;
		std::vector<Deserializer*>	mDeserializers;

	};


	IConditionSet& createConditionSet();
	RARetCode	   destroyConditionSet(CONDITION_SET_ID condSetId);

	ConditionSaver();
	virtual ~ConditionSaver();
private:
	uint32_t	mIdSource;

	std::vector<ConditionSet*>
				mCondSets;
};