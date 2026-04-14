#include "pch.h"
#include "rose_aura_test.h"

#include "RoseAura.h"
#include "RoseAuraReturnCode.h"
#include "raylib.h"

using namespace RoseAuraReturnCode;

class TestRenderer : public IGraphicsManager::IObjectRenderer {
public:

	void doPreprocess() {

	}

	void render()
	{
		DrawText("Close this window", 190, 200, 30, DARKGRAY);
	};

	virtual ~TestRenderer() = default;
	TestRenderer() = default;
};

TEST(testGraphicsManager, APITest)
{
	ROSE_AURA_TEST_BEGIN;
	{
		TestRenderer* testRenderer  = new TestRenderer();
		TestRenderer* testRenderer2 = new TestRenderer();

		std::unique_ptr<RoseAura> ra = RoseAura::create();

		IGraphicsManager& gm = ra->getGraphicsManager();

		EXPECT_EQ(gm.setRenderer(nullptr)			, RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(gm.setRenderer(testRenderer)		, RARetCode::RET_OK);
		EXPECT_EQ(gm.removeRenderer(nullptr)		, RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(gm.removeRenderer(testRenderer2)	, RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(gm.removeRenderer(testRenderer)	, RARetCode::RET_OK);

		EXPECT_EQ(gm.setRenderer(testRenderer), RARetCode::RET_OK);

		IGraphicsManager::Conf conf;
		conf.mWindowWidth  = 800;
		conf.mWindowHeight = 600;
		conf.mWindowTitle  = "testGraphicsManager";
		conf.mFrameRate    = 30;

		gm.runUntilClosed(conf);

		delete testRenderer;
		delete testRenderer2;

	}
	ROSE_AURA_TEST_FIN;
}