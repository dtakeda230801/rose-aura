#include "pch.h"
#include "rose_aura_test.h"

#include "RoseAura.h"
#include "RoseAuraReturnCode.h"

#include "Utility.h"

TEST(testConditionSaver, APITest)
{
	ROSE_AURA_TEST_BEGIN;
	{
		std::unique_ptr<RoseAura> ra = RoseAura::create();
		IConditionSaver&		  cs = ra->getConditionSaver();

		//////////////////////
		IConditionSaver::IConditionSet& condSet1
									 = cs.createConditionSet();

		Utility::printLog("Condition ID:%d", condSet1.getConditionId());

		EXPECT_EQ(cs.destroyConditionSet(condSet1.getConditionId()), RARetCode::RET_OK);
		//////////////////////
		IConditionSaver::IConditionSet& condSet2
									= cs.createConditionSet();

		Utility::printLog("Condition ID:%d",condSet2.getConditionId());

		condSet2.setConditionSetName("Test Condition Set");

		std::string ret;

		EXPECT_EQ(condSet2.setCondition("Test Condition","Green"), RARetCode::RET_OK);
		EXPECT_EQ(condSet2.getCondition("Test Condition", ret), RARetCode::RET_OK);
		EXPECT_TRUE(ret == "Green");
		EXPECT_EQ(condSet2.getCondition("Test Condition2", ret), RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(condSet2.setCondition("Test Condition", "Blue"), RARetCode::RET_OK);
		EXPECT_EQ(condSet2.getCondition("Test Condition", ret), RARetCode::RET_OK);
		EXPECT_TRUE(ret == "Blue");

		IConditionSaver::ISerializer& s = condSet2.getSerializer("Test Serialized Condition");
		std::any uint32Data1 = uint32_t(0x87654321);
		std::any uint32Data2 = uint32_t(0x12345678);
		std::any strData     = std::string("Test Serialized Data");

		s.serialize(uint32Data1);
		s.serialize(uint32Data2);
		s.serialize(strData);

		IConditionSaver::IDeserializer& d = condSet2.getDeserializer("Test Serialized Condition");
		std::any result;
		d.deserialize(typeid(uint32_t),result);
		EXPECT_EQ(std::any_cast<uint32_t>(result), 0x87654321);
		d.deserialize(typeid(uint32_t), result);
		EXPECT_EQ(std::any_cast<uint32_t>(result), 0x12345678);
		d.deserialize(typeid(std::string), result);
		EXPECT_EQ(std::any_cast<std::string>(result), "Test Serialized Data");

		//////////////////////
		EXPECT_EQ(condSet2.saveToFile("..\\..\\test\\rose_aura_test\\testResultExportedCond.json"), RARetCode::RET_OK);
		EXPECT_EQ(condSet2.loadFromFile("..\\..\\test\\rose_aura_test\\testResultExportedCond.json"), RARetCode::RET_OK);
		EXPECT_EQ(condSet2.getCondition("Test Condition", ret), RARetCode::RET_OK);
		EXPECT_TRUE(ret == "Blue");

		IConditionSaver::IDeserializer& d2 = condSet2.getDeserializer("Test Serialized Condition");
		d2.deserialize(typeid(uint32_t), result);
		EXPECT_EQ(std::any_cast<uint32_t>(result), 0x87654321);
		d2.deserialize(typeid(uint32_t), result);
		EXPECT_EQ(std::any_cast<uint32_t>(result), 0x12345678);
		d2.deserialize(typeid(std::string), result);
		EXPECT_EQ(std::any_cast<std::string>(result), "Test Serialized Data");

	}
	ROSE_AURA_TEST_FIN;
}