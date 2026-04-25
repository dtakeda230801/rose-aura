#include "pch.h"
#include "rose_aura_test.h"
#include "RoseAura.h"

TEST(testRoseAuraClass, APITest)
{
	ROSE_AURA_TEST_BEGIN;
	{
		std::unique_ptr<RoseAura> ra = RoseAura::create();

		ICentralLooper& cl = ra->getCentralLooper();
		IGraphicsManager& gm = ra->getGraphicsManager();
		IInputHandler& ih = ra->getInputHandler();
		IObjectActivator& ir = ra->getObjectActivator();
		IWorldNavigator& iw = ra->getWorldNavigator();
	}
	ROSE_AURA_TEST_FIN;
}