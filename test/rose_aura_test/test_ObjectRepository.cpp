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
	}

	virtual ~TestObj() = default;

private:
	int		mInt;
	IInputHandler& mInputHandler;
};


TEST(testObjectRepository, APITest)
{
	ROSE_AURA_TEST_BEGIN;
	{
		RARetCode ret;

		std::unique_ptr<RoseAura> ra = RoseAura::create();

		IObjectRepository& objR = ra->getObjectRepository();

		IObjectRepository::ObjectBinder binder = 
			objR.makeObjectBinder<TestObj, int>(&TestObj::init, &TestObj::term, 10, ra->getInputHandler());

		IObjectRepository::OBJECT_ID id1 = objR.registerObject(binder);
		EXPECT_EQ(id1,1);

		ret = objR.unregisterObject(id1);
		EXPECT_EQ(ret, RARetCode::RET_OK);

		ret = objR.unregisterObject(id1);
		EXPECT_EQ(ret, RARetCode::RET_ERR_NOT_FOUND);

		//////////////////////////////////
		IObjectRepository::TAG_ID TEST_TAG = 0x01;
		std::vector<IObjectRepository::TAG_ID> tags;
		tags.push_back(TEST_TAG);

		id1 = objR.registerObject(binder,tags);
		EXPECT_EQ(id1, 2);

	}
	ROSE_AURA_TEST_FIN;
}