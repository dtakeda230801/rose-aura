#include "pch.h"
#include "rose_aura_test.h"
#include "RoseAura.h"

class TestObj
{
public:
	void func() {
		mInt += 1;
	};

	TestObj(int in) :
		mInt(in)
	{
	}

	virtual ~TestObj() = default;

private:
	int		mInt;
};


TEST(testObjectRepository, APITest)
{
	ROSE_AURA_TEST_BEGIN;
	{
		std::unique_ptr<RoseAura> ra = RoseAura::create();

		IObjectRepository& objR = ra->getObjectRepository();

		IObjectRepository::ObjectBinder builder
			= objR.makeObjectBinder<TestObj, int>(10);

		TestObj* test = static_cast<TestObj*>(builder.create(builder.params));
		builder.destroy(test);

		builder.destroyParams(builder.params);

	}
	ROSE_AURA_TEST_FIN;
}