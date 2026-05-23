#pragma once

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#include "RoseAura.h"
#include "Tool.h"
#include "Utility.h"


namespace ModelCheckerObjects {

	//////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////
	class ModelViewer : public ICentralLooper::ITask
		              , public IGraphicsManager::IGraphicsRenderer
		              , public IInputHandler::IInputHandlerCallback
	{
	public:
		void doTask()
		{
			mStoryAnchor.changeState("Child", IStoryAnchor::StoryPointState::COMPLETED);
		}

		void onTaskFinish()
		{
		}

		std::string getTaskName()
		{
			return "ModelCheckerObjects#ModelViewer";
		}

		void preprocess()
		{
			if (!mInitialized) {
				Vector3 dir = { -1, -1, -1 };

				Shader* lightingShader = static_cast<Shader*>(mGraphicsManager.getShader(mLitingShaderId));

				int lightDirLoc = GetShaderLocation(*lightingShader, "lightDir");
				int lightColorLoc = GetShaderLocation(*lightingShader, "lightColor");
				int ambientLoc = GetShaderLocation(*lightingShader, "ambient");

				Vector3 lightDir   = Vector3Normalize({ 0.8f, 2.0f, 4.0f });
				Vector4 lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
				Vector4 ambient    = { 0.7f, 0.7f, 1.0f, 1.0f };

				SetShaderValue(*lightingShader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);
				SetShaderValue(*lightingShader, lightColorLoc, &lightColor, SHADER_UNIFORM_VEC4);
				SetShaderValue(*lightingShader, ambientLoc, &ambient, SHADER_UNIFORM_VEC4);

				mModel = LoadModel(MODEL_FILE);
				mModelAnimation = LoadModelAnimations(MODEL_FILE, &mAnimationNum);

				for (int i = 0; i < mModel.materialCount; ++i)
				{
					mModel.materials[i].shader = *lightingShader;
				}

				mInitialized = true;
				mInputEnable = true;
			}

			if (mExit && mInitialized) {
				UnloadModelAnimations(mModelAnimation, mAnimationNum);
				UnloadModel(mModel);
				if (mModel.currentPose) {
					free(mModel.currentPose);
					mModel.currentPose = nullptr;
				}
				if (mModel.boneMatrices) {
					free(mModel.boneMatrices);
					mModel.boneMatrices = nullptr;
				}

				mCentralLooper.enqueueTask(this);
				mInitialized = false;
			}
		}

		void render()
		{
			if (mInitialized) {

				BeginMode3D(mCamera);

				UpdateModelAnimation(mModel, mModelAnimation[mAnimationIndex], mFrame);

				rlDisableBackfaceCulling();
				DrawModel(mModel, { 0,0,0 }, 1.0f, WHITE);
				rlEnableBackfaceCulling();

				DrawGrid(10, 1.0f);

				EndMode3D();

				////////////////////////////
				Vector2     pos;
				std::string message;
				Font* font = static_cast<Font*>(mGraphicsManager.getDefaultFont());

				pos = { 10 , 10 };
				message = "frame : ";
				message += std::to_string(mFrame);

				DrawTextEx(*font, message.c_str(), pos, 25, 5, WHITE);

				pos.y += 25;
				message = "Number of Animation : ";
				message += std::to_string(mAnimationNum);

				DrawTextEx(*font, message.c_str(), pos, 25, 5, WHITE);

				pos.y += 25;
				message = "Current Animation : ";
				message += std::string(mModelAnimation[mAnimationIndex].name);

				DrawTextEx(*font, message.c_str(), pos, 25, 5, WHITE);

				pos.y += 25;
				message = "Number of Frame : ";
				message += std::to_string(mModelAnimation[mAnimationIndex].keyframeCount);

				DrawTextEx(*font, message.c_str(), pos, 25, 5, WHITE);

				pos = { 10 , 700 };
				message = "Change Animation";
				DrawTextEx(*font, message.c_str(), pos, 25, 5, WHITE);

				pos.y += 25;
				message = "Exit";
				DrawTextEx(*font, message.c_str(), pos, 25, 5, WHITE);

				int32_t forcusY = 25 * mCursor + 700 + 25;

				DrawLine(10, forcusY, 300, forcusY, { 123, 200, 50, 255 });
			}
		}

		void onInputEvent(std::vector<std::pair<InputState, InputType>>& events)
		{
			if (!mInputEnable) {
				return;
			}

			for (auto event : events) {
				InputState state = event.first;
				InputType  type = event.second;

				if (state == InputState::PUSHED) {
					if (type == InputType::UP) {
						--mCursor;
						if (mCursor < 0) {
							mCursor = 1;
						}
					} else if (type == InputType::DOWN) {
						++mCursor;
						if (mCursor >= 2) {
							mCursor = 0;
						}
					} else if (type == InputType::LEFT) {
						--mFrame;
						if (mFrame < 0) {
							mFrame = static_cast<float>(mModelAnimation[mAnimationIndex].keyframeCount);
						}
					} else if (type == InputType::RIGHT) {
						++mFrame;
						if (mFrame > mModelAnimation[mAnimationIndex].keyframeCount) {
							mFrame = 0;
						}
					} else if (type == InputType::ACTION1) {
						if (mCursor == 0) {
							++mAnimationIndex;
							if (mAnimationIndex == mAnimationNum) {
								mAnimationIndex = 0;
							}
						} else if (mCursor == 1) {
							mExit = true;
						}
						break;
					}
				}
			}
		}

		void init()
		{
			mLitingShaderId = mGraphicsManager.setShaderFile("resources\\lighting.vs", "resources\\lighting.fs");

			mInputHandler.registerCallback(this);
			mGraphicsManager.setRenderer(this);
		}

		void fin()
		{
			mGraphicsManager.removeRenderer(this);
			mInputHandler.unregisterCallback(this);

			mGraphicsManager.removeShader(mLitingShaderId);
		}

		ModelViewer(RoseAura& ra)
			: mCentralLooper(ra.getCentralLooper())
			, mGraphicsManager(ra.getGraphicsManager())
			, mInputHandler(ra.getInputHandler())
			, mStoryAnchor(ra.getStoryAnchor())
			, mCamera{}
			, mModel{}
			, mModelAnimation(nullptr)
			, mAnimationNum(0)
			, mInputEnable(false)
			, mInitialized(false)
			, mExit(false)
			, mAnimationIndex(0)
			, mFrame(0.0f)
			, mModelId(0)
			, mLitingShaderId(0)
			, mCursor(0)
		{
			mCamera.position   = { 1.0f, 2.5f, 3.0f };
			mCamera.target     = { 0.0f, 0.5f, 0.0f };
			mCamera.up         = { 0.0f, 1.0f, 0.0f };
			mCamera.fovy       = 45.0f;
			mCamera.projection = CAMERA_PERSPECTIVE;
		}

		virtual ~ModelViewer() = default;

	private:
		static constexpr const char* MODEL_FILE = "resources\\main.glb";

		ICentralLooper&		mCentralLooper;
		IGraphicsManager&	mGraphicsManager;
		IInputHandler&      mInputHandler;
		IStoryAnchor&		mStoryAnchor;

		Camera			    mCamera;

		Model			    mModel;
		ModelAnimation*		mModelAnimation;
		int32_t				mAnimationNum;

		int32_t				mAnimationIndex;
		float				mFrame;

		bool	            mInputEnable;
		bool                mInitialized;
		bool				mExit;

		int32_t				mCursor;

		IGraphicsManager::MODEL_ID
						    mModelId;

		IGraphicsManager::SHADER_ID
						    mLitingShaderId;

	};

	//////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////
	std::vector<IObjectActivator::OBJECT_ID> registerObjects(RoseAura& ra)
	{
		std::vector<IObjectActivator::OBJECT_ID>  ids;
		std::vector<IObjectActivator::TAG_ID>	  tags = { TAG_MODEL_CHECKER_OBJECT , TAG_CHILD_OBJECT };

		IObjectActivator& objectRepository = ra.getObjectActivator();

		//////////////////////////////////
		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<ModelViewer, RoseAura&>(
					&ModelViewer::init, &ModelViewer::fin, ra)
				, tags
			));

		return ids;
	}

}
