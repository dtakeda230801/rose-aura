#include <sstream>
#include <iomanip>
#include <memory>

#include "ConditionSaver.h"

#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;


///////////////////////////////////////////
///////////////////////////////////////////
IConditionSaver::IConditionSet& ConditionSaver::createConditionSet()
{
	ConditionSet* ins = new ConditionSet(++mIdSource);
	mCondSets.push_back(ins);
	return *ins;
}
RARetCode ConditionSaver::destroyConditionSet(CONDITION_SET_ID condSetId)
{
	for (auto ite = mCondSets.begin(); ite != mCondSets.end();) {
		ConditionSet* condSet = *ite;

		if (condSet->getConditionId() == condSetId) {
			delete condSet;
			mCondSets.erase(ite);
			return RARetCode::RET_OK;
		} else {
			++ite;
		}
	}
	return RARetCode::RET_ERR_INVALID_ARG;
}

ConditionSaver::ConditionSaver() :
	mIdSource(0)
{
}
ConditionSaver::~ConditionSaver()
{
	if (!mCondSets.empty()) {
		for (auto condSet : mCondSets) {
			delete condSet;
		}
	}
	mCondSets.clear();
}

///////////////////////////////////////////
///////////////////////////////////////////
IConditionSaver::CONDITION_SET_ID ConditionSaver::ConditionSet::getConditionId()
{
	return mId;
}

void ConditionSaver::ConditionSet::setConditionSetName(std::string condSetName)
{
	mCondSetName = condSetName;
}

RARetCode ConditionSaver::ConditionSet::setCondition(std::string condName, std::string value)
{
	for (auto& pair : mRegularConds) {
		if (pair.first == condName) {
			pair.second = value;
			return RARetCode::RET_OK;
		}
	}

	mRegularConds.emplace_back(std::pair<std::string, std::string>(condName, value));
	return RARetCode::RET_OK;
}

RARetCode ConditionSaver::ConditionSet::getCondition(std::string condName, std::string& value)
{
	for (auto& pair : mRegularConds) {
		if (pair.first == condName) {
			value = pair.second;
			return RARetCode::RET_OK;
		}
	}
	return RARetCode::RET_ERR_INVALID_ARG;
}

IConditionSaver::ISerializer& ConditionSaver::ConditionSet::getSerializer(std::string condName)
{
	Serializer* serializer = nullptr;

	for (auto& pair : mSerializedConds) {
		if (pair.first == condName) {
			serializer = new Serializer(*pair.second);
		}
	}

	if (!serializer) {
		std::pair<std::string, std::unique_ptr<std::string>>& newPair
			= mSerializedConds.emplace_back(
				std::pair<std::string, std::unique_ptr<std::string>>
				(condName, std::make_unique<std::string>("")) );
		serializer = new Serializer(*newPair.second);
	}

	mSerializers.push_back(serializer);
	return *serializer;
}

IConditionSaver::IDeserializer& ConditionSaver::ConditionSet::getDeserializer(std::string condName)
{
	Deserializer* deserializer = nullptr;

	for (auto& pair : mSerializedConds) {
		if (pair.first == condName) {
			deserializer = new Deserializer(*pair.second);
		}
	}

	if (!deserializer) {
		std::pair<std::string, std::unique_ptr<std::string>>& newPair
			= mSerializedConds.emplace_back(
				std::pair<std::string, std::unique_ptr<std::string>>
				(condName, std::make_unique<std::string>("")));
		deserializer = new Deserializer(*newPair.second);
	}

	mDeserializers.push_back(deserializer);
	return *deserializer;

}

RARetCode ConditionSaver::ConditionSet::saveToFile(const char* filename)
{
	json j;

	j["condition_set_name"]    = mCondSetName;
	j["regular_conditions"]    = json::array();
	j["serialized_conditions"] = json::array();

	for (auto& cond : mRegularConds) {
		j["regular_conditions"].push_back({ {"key",cond.first},{"value",cond.second}});
	}

	for (auto& cond : mSerializedConds) {
		j["serialized_conditions"].push_back({ {"key",cond.first},{"value",*cond.second} });
	}

	std::ofstream ofs(filename);
	if (!ofs) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}
	ofs << j.dump(4);

	return RARetCode::RET_OK;
}
RARetCode ConditionSaver::ConditionSet::loadFromFile(const char* filename)
{
	std::ifstream ifs(filename);
	if (!ifs) {
		return RARetCode::RET_ERR_INVALID_ARG;
	}

	json j = json::parse(ifs);

	mRegularConds.clear();
	mSerializedConds.clear();

	mCondSetName = j["condition_set_name"];

	for (const auto& cond : j["regular_conditions"]) {
		std::string key   = cond["key"];
		std::string value = cond["value"];

		mRegularConds.emplace_back(std::pair<std::string, std::string>(key, value));
	}

	for (const auto& cond : j["serialized_conditions"]) {
		std::string key   = cond["key"];
		std::string value = cond["value"];

		mSerializedConds.emplace_back(
			std::pair<std::string, std::unique_ptr<std::string>>
			(key, std::make_unique<std::string>(value)));
	}

	return RARetCode::RET_OK;
}

ConditionSaver::ConditionSet::ConditionSet(CONDITION_SET_ID id) :
	mId(id)
	, mCondSetName("")
{
}

ConditionSaver::ConditionSet::~ConditionSet()
{
	for (auto serializer : mSerializers) {
		delete serializer;
	}

	for (auto deserializer : mDeserializers) {
		delete deserializer;
	}
}



///////////////////////////////////////////
///////////////////////////////////////////
RARetCode ConditionSaver::Serializer::serialize(std::any data)
{
	RARetCode ret = RARetCode::RET_OK;

	if (auto p = std::any_cast<uint32_t>(&data)){
		std::stringstream ss;
		ss << std::hex << *p;
		mContainer += ss.str();
	} else if (auto p = std::any_cast<std::string>(&data)) {
		std::stringstream ss;
		const char* cStr = p->c_str();
		uint32_t    size = p->size();
		ss << std::hex << std::setw(8) << std::setfill('0') << size;
		ss << std::hex << cStr;
		mContainer += ss.str();
	} else {
		ret = RARetCode::RET_ERR_INVALID_PARAMS;
	}
	return ret;
}

ConditionSaver::Serializer::Serializer(std::string& container) :
	mContainer(container)
{
}


///////////////////////////////////////////
///////////////////////////////////////////
RARetCode ConditionSaver::Deserializer::deserialize(std::type_index type, std::any& out)
{
	RARetCode ret = RARetCode::RET_OK;

	if (type == typeid(uint32_t)) {
		std::stringstream ss;
		ss << (mContainer).c_str();
		std::string uint32Str = ss.str().substr(mPointer, 8);
		uint32_t uint32val = std::stoul(uint32Str, nullptr, 16);
		out = uint32val;
		mPointer += 8;
	} else if (type == typeid(std::string)) {
		std::string result;
		std::stringstream ss;
		ss << (mContainer).c_str();
		std::string sizeStr = ss.str().substr(mPointer, 8);
		uint32_t size       = std::stoul(sizeStr, nullptr, 16);
		mPointer += 8;
		result = ss.str().substr(mPointer, size);
		mPointer += size;
		out = result;
	} else {
		ret = RARetCode::RET_ERR_INVALID_PARAMS;
	}
	return ret;
}


ConditionSaver::Deserializer::Deserializer(std::string& container) :
	  mContainer(container)
	, mPointer(0)
{
}
