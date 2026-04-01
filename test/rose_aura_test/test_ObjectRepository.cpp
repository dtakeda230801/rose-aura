#include "pch.h"
#include "rose_aura_test.h"
#include "RoseAura.h"

#include "Utility.h"

class TestObj
{
public:


	void func() {
		mInt += 1;
	};

	void init() {
		Utility::printLog("call init");
	};

	void term() {
		Utility::printLog("call term");
	};

	///////////////////////
	TestObj(int in, IInputHandler& ih) :
		 mInt(in)
		,mInputHandler(ih)
	{
		Utility::printLog("constructed");
	}

	virtual ~TestObj() {
		Utility::printLog("destructed");
	}

private:
	int		mInt;
	IInputHandler& mInputHandler;
};


TEST(testObjectRepository, APITest)
{
	ROSE_AURA_TEST_BEGIN;
	{
		RARetCode ret;
		bool	  check;

		std::vector<IObjectRepository::TAG_ID> tags;

		IObjectRepository::OBJECT_ID id1;
		IObjectRepository::OBJECT_ID id2;

		IObjectRepository::TAG_ID TEST_TAG1 = 0x01;
		IObjectRepository::TAG_ID TEST_TAG2 = 0x02;

		//////////////////////////////////
		std::unique_ptr<RoseAura> ra = RoseAura::create();

		IObjectRepository& objR = ra->getObjectRepository();

		//////////////////////////////////
		id1 = objR.registerObject(
			objR.makeObjectBinder<TestObj, int, IInputHandler&>(
				&TestObj::init
				, &TestObj::term
				, 10
				, ra->getInputHandler() )
			);
		EXPECT_EQ(id1,1);

		ret = objR.unregisterObject(id1);
		EXPECT_EQ(ret, RARetCode::RET_OK);

		ret = objR.unregisterObject(id1);
		EXPECT_EQ(ret, RARetCode::RET_ERR_NOT_FOUND);

		//////////////////////////////////
		id1 = objR.registerObject(
			objR.makeObjectBinder<TestObj, int, IInputHandler&>(
				  &TestObj::init
				, &TestObj::term
				, 10
				, ra->getInputHandler()	)
			);
		EXPECT_EQ(id1, 2);

		ret = objR.addTag(id1, TEST_TAG1);
		EXPECT_EQ(ret, RARetCode::RET_OK);

		ret = objR.addTag(0, TEST_TAG1);
		EXPECT_EQ(ret, RARetCode::RET_ERR_NOT_FOUND);

		ret = objR.removeTag(0, TEST_TAG1);
		EXPECT_EQ(ret, RARetCode::RET_ERR_NOT_FOUND);

		ret = objR.removeTag(id1, TEST_TAG1);
		EXPECT_EQ(ret, RARetCode::RET_OK);

		//////////////////////////////////
		check = objR.isActivate(id1);
		EXPECT_FALSE(check);

		ret = objR.activate(id1);
		EXPECT_EQ(ret, RARetCode::RET_OK);

		ret = objR.activate(id1);
		EXPECT_EQ(ret, RARetCode::RET_ERR_INVALID_STATE);

		ret = objR.activate(0);
		EXPECT_EQ(ret, RARetCode::RET_ERR_NOT_FOUND);

		check = objR.isActivate(id1);
		EXPECT_TRUE(check);

		check = objR.isActivate(0);
		EXPECT_FALSE(check);

		ret = objR.deactivate(id1);
		EXPECT_EQ(ret, RARetCode::RET_OK);

		ret = objR.deactivate(id1);
		EXPECT_EQ(ret, RARetCode::RET_ERR_INVALID_STATE);

		check = objR.isActivate(id1);
		EXPECT_FALSE(check);

		//////////////////////////////////
		tags.push_back(TEST_TAG1);
		tags.push_back(TEST_TAG2);

		ret = objR.addTag(id1, TEST_TAG1);
		EXPECT_EQ(ret, RARetCode::RET_OK);

		IObjectRepository::OBJECT_ID id2 = objR.registerObject(
			objR.makeObjectBinder<TestObj, int, IInputHandler&>(
				  &TestObj::init
				, &TestObj::term
				, 10
				, ra->getInputHandler()	)
			, tags
			);
		EXPECT_EQ(id2, 3);

		check = objR.isActivateByTag(TEST_TAG1);
		EXPECT_FALSE(check);

		ret = objR.activateByTag(TEST_TAG1);
		EXPECT_EQ(ret, RARetCode::RET_OK);

		ret = objR.activateByTag(TEST_TAG1);
		EXPECT_EQ(ret, RARetCode::RET_ERR_INVALID_STATE);

		check = objR.isActivateByTag(TEST_TAG1);
		EXPECT_TRUE(check);

		ret = objR.deactivateByTag(TEST_TAG1);
		EXPECT_EQ(ret, RARetCode::RET_OK);

		ret = objR.deactivateByTag(TEST_TAG1);
		EXPECT_EQ(ret, RARetCode::RET_ERR_INVALID_STATE);

		//////////////////////////////////
		ret = objR.activateByTag(TEST_TAG2);
		EXPECT_EQ(ret, RARetCode::RET_OK);

		check = objR.isActivateByTag(TEST_TAG2);
		EXPECT_TRUE(check);

		check = objR.isActivateByTag(TEST_TAG1);
		EXPECT_FALSE(check);

		ret = objR.deactivateByTag(TEST_TAG2);
		EXPECT_EQ(ret, RARetCode::RET_OK);
	}
	ROSE_AURA_TEST_FIN;
}