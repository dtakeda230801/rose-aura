#pragma once

#include <mutex>

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#include "RoseAura.h"
#include "RoseAuraReturnCode.h"
#include "GeometricalUtility.h"
#include "MediaUtility.h"
#include "Utility.h"

#include "Game.h"

using namespace RoseAuraMediaUtility;

#define TXT_POS_X	10
#define TXT_POS_Y	10
#define CIRCLE_SIZE 20
#define MOVE_DELTA  5
#define MOVE_RATE   100.0f

namespace GameObjects {

	IWorldNavigator::WorldConfig gWorldConf;

	void buildWorldConf()
	{
		gWorldConf.mWorldSpace.mMin.mX = -500;
		gWorldConf.mWorldSpace.mMin.mY = -100;
		gWorldConf.mWorldSpace.mMin.mZ = -500;
		gWorldConf.mWorldSpace.mMax.mX =  500;
		gWorldConf.mWorldSpace.mMax.mY =  100;
		gWorldConf.mWorldSpace.mMax.mZ =  500;

		gWorldConf.mActiveRange.mX = 150;
		gWorldConf.mActiveRange.mY = 100;
		gWorldConf.mActiveRange.mZ = 100;

		gWorldConf.mNonScrollRange.mX = 100;
		gWorldConf.mNonScrollRange.mY = 50;
		gWorldConf.mNonScrollRange.mZ = 50;

		gWorldConf.mPrimaryEntityPosition.mX = 0;
		gWorldConf.mPrimaryEntityPosition.mY = 0;
		gWorldConf.mPrimaryEntityPosition.mZ = 0;

		gWorldConf.mEnableFollowing = true;
		gWorldConf.mLimitScrolling  = true;
	}

	//////////////////////////////////////////////////////////////
	class MyCharacter
		: public IGraphicsManager::IGraphicsRenderer
		, public IInputHandler::IInputHandlerCallback

	{
	public:

		void onInputEvent(std::vector<std::pair<InputState, InputType>>& events)
		{
			if (!mInitialized) {
				return;
			}

			IWorldNavigator::Vec3 pos = mWorldNavigator.getPrimaryEntityPosition();
			IWorldNavigator::Vec3 previous = pos;

			for (auto event : events) {
				InputState state = event.first;
				InputType  type = event.second;

				if (state == InputState::PUSHED || state == InputState::PRESSED) {
					if (type == InputType::UP) {
						pos.mZ += MOVE_DELTA;
						changeAnimationIfNeeded(1);
					}
					else if (type == InputType::DOWN) {
						pos.mZ -= MOVE_DELTA;
						changeAnimationIfNeeded(1);
					}
					else if (type == InputType::LEFT) {
						pos.mX += MOVE_DELTA;
						changeAnimationIfNeeded(1);
					}
					else if (type == InputType::RIGHT) {
						pos.mX -= MOVE_DELTA;
						changeAnimationIfNeeded(1);
					}
					else if (type == InputType::ACTION1 && state != InputState::PRESSED) {
						mRestart = true;
						changeAnimationIfNeeded(0);
					}
				}
			}
			mWorldNavigator.movePrimaryEntityPosition(pos);

			if (!GeometricalUtility::equalVec3(pos, previous)) {
				int32_t deltaX = pos.mX - previous.mX;
				int32_t deltaZ = pos.mZ - previous.mZ;

				mTargetAngle = GeometricalUtility::calcAngle(static_cast<float>(deltaX), static_cast<float>(deltaZ));

				if (mTargetAngle < 0.0f) {
					mTargetAngle += 360.0f;
				}
				mWalking = true;

				mCamera.position = { static_cast<float>(pos.mX) / MOVE_RATE , 4.0f,static_cast<float>(pos.mZ) / MOVE_RATE - 4.0f };
				mCamera.target = { static_cast<float>(pos.mX) / MOVE_RATE , 1.0f, static_cast<float>(pos.mZ) / MOVE_RATE };

			}
			else {
				mWalking = false;
			}
		}


		void preprocess()
		{
			if (!mInitialized) {

				Vector3 dir = { -1, -1, -1 };

				Shader* lightingShader = static_cast<Shader*>(mGraphicsManager.getShader(mLitingShaderId));

				int lightDirLoc   = GetShaderLocation(*lightingShader, "lightDir");
				int lightColorLoc = GetShaderLocation(*lightingShader, "lightColor");
				int ambientLoc    = GetShaderLocation(*lightingShader, "ambient");

				Vector3 lightDir   = Vector3Normalize({ 0.8f, 2.0f, 4.0f });
				Vector4 lightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
				Vector4 ambient    = { 1.0f, 1.0f, 1.0f, 1.0f };

				SetShaderValue(*lightingShader, lightDirLoc,   &lightDir,   SHADER_UNIFORM_VEC3);
				SetShaderValue(*lightingShader, lightColorLoc, &lightColor, SHADER_UNIFORM_VEC4);
				SetShaderValue(*lightingShader, ambientLoc,    &ambient,    SHADER_UNIFORM_VEC4);

				changeAnimationIfNeeded(1);
				Model* model = static_cast<Model*>(mModelWrapper->getModel());

				for (int i = 0; i < model->materialCount; ++i) {
					model->materials[i].shader = *lightingShader;
				}

				mInitialized = true;
			}
		}

		void render()
		{
			if (mInitialized) {
				BeginMode3D(mCamera);

				bool forward = mWalking || mRestart;

				if (mRestart) {
					mRestart = false;
				}

				Model* model = static_cast<Model*>(mModelWrapper->getAnimetionModel(forward));

				IWorldNavigator::Vec3 pos = mWorldNavigator.getPrimaryEntityPosition();

				Vector3 rVec3 = { static_cast<float>(pos.mX) / MOVE_RATE, static_cast<float>(pos.mY) / MOVE_RATE, static_cast<float>(pos.mZ) / MOVE_RATE };


				if (mTurnFrame > 0 || mTargetAngle != mAngle) {

					if (mTurnFrame > 0 && mTargetAngle != mPreviousTargetAngle) {
						mTurnFrame = 0;
					}

					if (mTurnFrame == 0) {
						mTurnFrame = 3;
						mPreviousTargetAngle = mTargetAngle;

						mDeltaAngle = mTargetAngle - mAngle;
						if (std::abs(mDeltaAngle) > 180.0f) {
							mDeltaAngle = (360.0f - std::abs(mDeltaAngle)) * (mDeltaAngle / std::abs(mDeltaAngle)) * -1.0f;
						}
					}
					mAngle = std::round(mAngle + mDeltaAngle / 3.0f);
					if (mAngle < 0.0f) {
						mAngle += 360.0f;
					}
					--mTurnFrame;
				}

				rlDisableBackfaceCulling();
				DrawModelEx(*model, rVec3, { 0,1,0 }, mAngle, { 1,1,1 }, WHITE);
				rlEnableBackfaceCulling();

				DrawGrid(10, 1.0f);

				EndMode3D();
			}
		};

		void init()
		{
			mLitingShaderId = mGraphicsManager.setShaderFile("resources\\lighting.vs", "resources\\lighting.fs");
			mModelId        = mGraphicsManager.setModel("resources\\main2.glb", true);
			mModelWrapper   = mGraphicsManager.getModelWrapper(mModelId);
			mGraphicsManager.setRenderer(this);
			mInputHndler.registerCallback(this);
		}

		void fin()
		{
			mInputHndler.unregisterCallback(this);
			mGraphicsManager.removeRenderer(this);
			mGraphicsManager.releaseModelWrapper(mModelId);
			mGraphicsManager.removeShader(mLitingShaderId);
		}

		MyCharacter(RoseAura& ra)
			: mCentralLooper(ra.getCentralLooper())
			, mGraphicsManager(ra.getGraphicsManager())
			, mInputHndler(ra.getInputHandler())
			, mWorldNavigator(ra.getWorldNavigator())
			, mModelId(0)
			, mLitingShaderId(0)
			, mViewPosLoc(0)
			, mCurrentAnimation(0)
			, mRestart(false)
			, mModelWrapper(nullptr)
			, mInitialized(false)
			, mWalking(false)
			, mAngle(0.0f)
			, mTargetAngle(0.0f)
			, mPreviousTargetAngle(0.0f)
			, mDeltaAngle(0.0f)
			, mTurnFrame(false)
		{
			mCamera.position = { 0.0f, 4.0f,-4.0f };
			mCamera.target = { 0.0f, 1.0f, 0.0f };
			mCamera.up = { 0.0f, 1.0f, 0.0f };
			mCamera.fovy = 45.0f;
			mCamera.projection = CAMERA_PERSPECTIVE;
		}

		virtual ~MyCharacter() = default;

	private:
		void changeAnimationIfNeeded(uint32_t next)
		{
			if (mCurrentAnimation != next) {
				mCurrentAnimation = next;

				mModelWrapper->selectAndResetAnimation(mCurrentAnimation);

				switch (next) {
				case 0:
				{
					std::vector<uint32_t> stable = { 73 };
					mModelWrapper->setAdjustment(0, 0, 0.3f, stable);
					break;
				}
				case 1:
				{
					std::vector<uint32_t> stable = { 3,31 };
					mModelWrapper->setAdjustment(3, 3, 0.3f, stable);
					break;
				}

				default:
					break;
				}
			}
		}

		ICentralLooper&    mCentralLooper;
		IGraphicsManager&  mGraphicsManager;
		IInputHandler&     mInputHndler;
		IWorldNavigator&   mWorldNavigator;
		Camera			   mCamera;

		IGraphicsManager::MODEL_ID
			               mModelId;

		IGraphicsManager::SHADER_ID
			               mLitingShaderId;
		int32_t			   mViewPosLoc;

		IGraphicsManager::IModelWrapper*
						   mModelWrapper;
		uint32_t           mCurrentAnimation;

		bool               mRestart;
		bool			   mInitialized;
		bool			   mWalking;

		float			   mAngle;
		float              mTargetAngle;
		float              mPreviousTargetAngle;
		float              mDeltaAngle;
		uint32_t           mTurnFrame;
	};

	//////////////////////////////////////////////////////////////
	class SoundEffect01 : public IInputHandler::IInputHandlerCallback
	{
	public:
		//IInputHandlerCallback
		void onInputEvent(std::vector<std::pair<InputState, InputType>>& events)
		{
			for (auto event : events) {
				InputState state = event.first;
				InputType  type  = event.second;

				if (state == InputState::PUSHED && type == InputType::ACTION2) {
					mSoundSnapshotRenderer->playSound();
				}
			}
		}

		void init()
		{
			IInputHandler& ih = mRa.getInputHandler();
			ih.registerCallback(this);
			mSoundSnapshotRenderer = new SoundSnapshotRenderer(mRa,"resources\\effect001.wav");
		}

		void fin()
		{
			delete mSoundSnapshotRenderer;
			IInputHandler& ih = mRa.getInputHandler();
			ih.unregisterCallback(this);
		}

		SoundEffect01(RoseAura& ra) :
			  mRa(ra)
			, mSoundSnapshotRenderer(nullptr)
		{
		}
		virtual ~SoundEffect01() = default;

	private:
		RoseAura&				mRa;
		SoundSnapshotRenderer*  mSoundSnapshotRenderer;
							
	};

	//////////////////////////////////////////////////////////////
	class Music : public IInputHandler::IInputHandlerCallback
	{
	public:
		//IInputHandlerCallback
		void onInputEvent(std::vector<std::pair<InputState, InputType>>& events)
		{
			for (auto event : events) {
				InputState state = event.first;
				InputType  type = event.second;

				if (state == InputState::PUSHED && type == InputType::ACTION3) {

					if (!mPlay) {
						mPlay = true;
						mMusicRenderer->playMusic();
					}
					else {
						mPlay = false;
						mMusicRenderer->stopPlaying();
					}
				}
			}
		}

		void init()
		{
			mMusicRenderer = new MusicRenderer(mRa, "resources\\Seeker.opus");
			mMusicRenderer->setJumpPoint(6101808, 2602800);

			IInputHandler& ih = mRa.getInputHandler();
			ih.registerCallback(this);
		}

		void fin()
		{
			mMusicRenderer->stopPlaying();
			delete mMusicRenderer;
			IInputHandler& ih = mRa.getInputHandler();
			ih.unregisterCallback(this);
		}

		Music(RoseAura& ra) :
			mRa(ra)
			, mMusicRenderer(nullptr)
			, mPlay(false)
		{
		}
		virtual ~Music() = default;

	private:
		RoseAura&		   mRa;
		MusicRenderer*	   mMusicRenderer;
		bool			   mPlay;
	};

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	std::vector<IObjectActivator::OBJECT_ID> registerObjects(RoseAura& ra)
	{
		std::vector<IObjectActivator::OBJECT_ID> ids;
		std::vector<IObjectActivator::TAG_ID>	  tags = { TAG_GAME_OBJECT };

		IObjectActivator& objectRepository = ra.getObjectActivator();

		//////////////////////////////////
		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<MyCharacter, RoseAura&>(
					&MyCharacter::init, &MyCharacter::fin, ra)
				, tags
			));

		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<SoundEffect01, RoseAura&>(
					&SoundEffect01::init, &SoundEffect01::fin, ra)
				, tags
			));

		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<Music, RoseAura&>(
					&Music::init, &Music::fin, ra)
				, tags
			));

		return ids;
	}

}
