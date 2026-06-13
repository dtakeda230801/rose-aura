
#include "pch.h"
#include "rose_aura_test.h"

#include "RoseAura.h"
#include "RoseAuraReturnCode.h"
#include "raylib.h"
#include "rlgl.h"

#include "res.h"

//#define RAYLIB_DEBUG

#ifdef RAYLIB_DEBUG
extern "C" __declspec(dllimport)
void __stdcall OutputDebugStringA(const char* lpOutputString);
#endif 

static bool gLoadAnimation = false;

static bool gInitialized = false;
static bool gRetOK       = false;

static IGraphicsManager::SHADER_ID gShaderId1 = 0;
static IGraphicsManager::SHADER_ID gShaderId2 = 0;

static IGraphicsManager::FONT_ID   gFontId    = 0;

static IGraphicsManager::MODEL_ID  gModelId   = 0;

class TestRenderer : public IGraphicsManager::IGraphicsRenderer {
public:

	void preprocess()
	{
		if (!gInitialized) {

			gRetOK = true;

			if (!mGraphicsManager.getShader(gShaderId1)) {
				Utility::printLog("Can not get sharder1");
				gRetOK = false;
			}

			if (!mGraphicsManager.getShader(gShaderId2)) {
				Utility::printLog("Can not get sharder2");
				gRetOK = false;
			}

			if (!mGraphicsManager.getFont(gFontId)) {
				Utility::printLog("Can not get font");
				gRetOK = false;
			}

			IGraphicsManager::IModelWrapper* modelWrapper
				= mGraphicsManager.getModelWrapper(gModelId);

			if (!modelWrapper->getModel()) {
				Utility::printLog("Can not get model");
				gRetOK = false;
			}

			if (gLoadAnimation) {

				if (RARetCode::RET_ERR_INVALID_ARG != modelWrapper->selectAndResetAnimation(10)) {
					Utility::printLog("selectAnimation return code is invalid");
					gRetOK = false;
				}

				if (RARetCode::RET_OK != modelWrapper->selectAndResetAnimation(0)) {
					Utility::printLog("selectAnimation return code is invalid");
					gRetOK = false;
				}

				Utility::printLog("getAnimationNum : %d", modelWrapper->getAnimationNum());


				if (!modelWrapper->getAnimetionModel(true)) {
					Utility::printLog("Can not get animation model");
					gRetOK = false;
				}
			}

			gInitialized = true;
		}

	}

	void render()
	{
		DrawText("Close this window", 190, 200, 30, DARKGRAY);
	};



	TestRenderer(IGraphicsManager& gm)
		: mGraphicsManager(gm)
	{
	}

	virtual ~TestRenderer() = default;

private:
	IGraphicsManager& mGraphicsManager;
};

TEST(testGraphicsManager, APITest)
{
#ifdef RAYLIB_DEBUG
	SetTraceLogLevel(LOG_DEBUG);

	SetTraceLogCallback(
		[](int level, const char* text, va_list args)
		{
			char buffer[2048];
			vsnprintf(buffer, sizeof(buffer), text, args);

			OutputDebugStringA(buffer);
			OutputDebugStringA("\n");
		});
#endif

	ROSE_AURA_TEST_BEGIN;
	{
		std::unique_ptr<RoseAura> ra = RoseAura::create();

		IGraphicsManager& gm = ra->getGraphicsManager();

		TestRenderer* testRenderer  = new TestRenderer(gm);
		TestRenderer* testRenderer2 = new TestRenderer(gm);

		EXPECT_EQ(gm.setRenderer(nullptr, IGraphicsManager::Layer::L_FRONT)	     , RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(gm.setRenderer(testRenderer, IGraphicsManager::Layer::L_FRONT) , RARetCode::RET_OK);
		EXPECT_EQ(gm.removeRenderer(nullptr)		                             , RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(gm.removeRenderer(testRenderer2)	                             , RARetCode::RET_ERR_INVALID_ARG);
		EXPECT_EQ(gm.removeRenderer(testRenderer)	                             , RARetCode::RET_OK);

		EXPECT_EQ(gm.setRenderer(testRenderer,IGraphicsManager::Layer::L_FRONT), RARetCode::RET_OK);

		std::string vsfile = "..\\..\\resources\\shader\\ConvertPictureV.fs";
		std::string fsfile = "..\\..\\resources\\shader\\ConvertPictureF.fs";

		RoseAuraResources::create();

		gShaderId1 = gm.setShaderFile(vsfile, fsfile);
		gShaderId2 = gm.setShader(RESOURCES->Res_ConvertPictureV, RESOURCES->Res_ConvertPictureF);
		gFontId    = gm.setFont("..\\..\\test\\rose_aura_test\\NotoSerifJP-Regular.ttf");

		gModelId   = gm.setModel("..\\..\\dummy_game\\DummyGame\\main.glb", gLoadAnimation);

		IGraphicsManager::Conf conf;
		conf.mWindowWidth  = 800;
		conf.mWindowHeight = 600;
		conf.mWindowTitle  = "testGraphicsManager";
		conf.mFrameRate    = 24;

		gm.runUntilClosed(conf);

		gm.releaseModelWrapper(gModelId);

		EXPECT_TRUE(gRetOK);

		delete testRenderer;
		delete testRenderer2;

	}
	ROSE_AURA_TEST_FIN;
}