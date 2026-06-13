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

			IWorldNavigator::Vec3 pos = RA_WORLD_NAVIGATOR.getPrimaryEntityPosition();
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
			RA_WORLD_NAVIGATOR.movePrimaryEntityPosition(pos);

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

				RA_GRAPHICS_MANAGER.updateCamera(&mCamera);

			}
			else {
				mWalking = false;
			}
		}


		void preprocess()
		{
			if (!mInitialized) {

				Vector3 dir = { -1, -1, -1 };

				Shader* lightingShader = static_cast<Shader*>(RA_GRAPHICS_MANAGER.getShader(mLitingShaderId));

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
				bool forward = mWalking || mRestart;

				if (mRestart) {
					mRestart = false;
				}

				Model* model = static_cast<Model*>(mModelWrapper->getAnimetionModel(forward));

				IWorldNavigator::Vec3 pos = RA_WORLD_NAVIGATOR.getPrimaryEntityPosition();

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
			}
		};

		void init()
		{
			mLitingShaderId = RA_GRAPHICS_MANAGER.setShaderFile("resources\\lighting.vs", "resources\\lighting.fs");
			mModelId        = RA_GRAPHICS_MANAGER.setModel("resources\\main2.glb", true);
			mModelWrapper   = RA_GRAPHICS_MANAGER.getModelWrapper(mModelId);
			RA_GRAPHICS_MANAGER.updateCamera(&mCamera);
			RA_GRAPHICS_MANAGER.setRenderer(this, IGraphicsManager::Layer::L_3D);
			RA_INPUT_HANDLER.registerCallback(this);
		}

		void fin()
		{
			RA_INPUT_HANDLER.unregisterCallback(this);
			RA_GRAPHICS_MANAGER.removeRenderer(this);
			RA_GRAPHICS_MANAGER.releaseModelWrapper(mModelId);
			RA_GRAPHICS_MANAGER.removeShader(mLitingShaderId);
		}

		MyCharacter()
			: mModelId(0)
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
			mSoundSnapshotRenderer = new SoundSnapshotRenderer(RA_INSTANCE,"resources\\effect001.wav");
			RA_INPUT_HANDLER.registerCallback(this);
		}

		void fin()
		{
			RA_INPUT_HANDLER.unregisterCallback(this);
			delete mSoundSnapshotRenderer;
		}

		SoundEffect01()
			: mSoundSnapshotRenderer(nullptr)
		{
		}
		virtual ~SoundEffect01() = default;

	private:
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
			mMusicRenderer = new MusicRenderer(RA_INSTANCE, "resources\\Seeker.opus");
			mMusicRenderer->setJumpPoint(6101808, 2602800);
			RA_INPUT_HANDLER.registerCallback(this);
		}

		void fin()
		{
			RA_INPUT_HANDLER.unregisterCallback(this);
			mMusicRenderer->stopPlaying();
			delete mMusicRenderer;
		}

		Music()
			: mMusicRenderer(nullptr)
			, mPlay(false)
		{
		}
		virtual ~Music() = default;

	private:
		MusicRenderer*	   mMusicRenderer;
		bool			   mPlay;
	};

	class Cards 
		: public IGraphicsManager::IGraphicsRenderer
		, public IInputHandler::IInputHandlerCallback
	{
	public:
		void onInputEvent(std::vector<std::pair<InputState, InputType>>& events)
		{
			for (auto event : events) {
				InputState state = event.first;
				InputType  type = event.second;

				if (state == InputState::PUSHED && type == InputType::ACTION4) {
					if (mStatus == Status::S_APPEAR) {
						mStatus = Status::S_TO_DISAPPEAR;
						prepareAnimation();

					} else if (mStatus == Status::S_DISAPPEAR){
						mStatus = Status::S_TO_APPEAR;
						prepareAnimation();
					}
				}
			}
		}

		void preprocess()
		{
			if (!mInitialized) {
				for (uint32_t i = 0; i < CARD_NUM; ++i) {
					mTextures.push_back(LoadTexture("resources\\card.png"));
				}

				mSrcRectangle.x = 0.0f;
				mSrcRectangle.y = 0.0f;
				mSrcRectangle.width = static_cast<float>(mTextures[0].width);
				mSrcRectangle.height = static_cast<float>(mTextures[0].height);

				mDstRectangle.x = static_cast<float>(WIN_SIZE_W) - 20.0f - 120.0f;
				mDstRectangle.y = 20.0f;
				mDstRectangle.width = 120.0f;
				mDstRectangle.height = 160.0f;

				mInitialized = true;
			}
		}

		void render()
		{
			if (mInitialized && mStatus != Status::S_DISAPPEAR) {

				for (uint32_t i = 0; i < CARD_NUM; i++) {
					Vector2   origin;
					Rectangle dst;

					origin.x = 0.0f;
					origin.y = 0.0f;

					dst   = mDstRectangle;
					dst.x = mDstRectangle.x - static_cast<float>(i) * (mDstRectangle.width + 10.0f);

					if (mStatus == Status::S_TO_APPEAR) {
						++mFrame;
						float startX = static_cast<float>(WIN_SIZE_W);
						float delta = static_cast<float>(mFrame) / static_cast<float>(ANIMATION_LEN);

						dst.x = startX - (startX - dst.x) * delta;

						if (mFrame == ANIMATION_LEN) {
							mStatus = Status::S_APPEAR;
						}

					} else if (mStatus == Status::S_TO_DISAPPEAR) {
						++mFrame;
						float targetX = static_cast<float>(WIN_SIZE_W);
						float delta   = static_cast<float>(mFrame) / static_cast<float>(ANIMATION_LEN);

						dst.x = dst.x + (targetX - dst.x) * delta;

						if (mFrame == ANIMATION_LEN) {
							mStatus = Status::S_DISAPPEAR;
						}
					}

					DrawTexturePro(mTextures[i], mSrcRectangle, dst, origin, 0.0f, mColor);
				}
			}
		}

		void prepareAnimation()
		{
			mFrame = 0;
		}

		void init()
		{
			RA_GRAPHICS_MANAGER.setRenderer(this, IGraphicsManager::Layer::L_FRONT);
			RA_INPUT_HANDLER.registerCallback(this);
		}

		void fin()
		{
			RA_INPUT_HANDLER.unregisterCallback(this);
			RA_GRAPHICS_MANAGER.removeRenderer(this);
		}

		Cards()
			: mInitialized(false)
			, mSrcRectangle{}
			, mDstRectangle{}
			, mColor{ 255,255,255,255 }
			, mStatus(Status::S_DISAPPEAR)
			, mFrame(0)
		{
		}

		virtual ~Cards() = default;

	private:
		static constexpr uint32_t CARD_NUM      = 6;
		static constexpr uint32_t ANIMATION_LEN = 12;

		enum class Status {
			  S_APPEAR
			, S_TO_APPEAR
			, S_DISAPPEAR
			, S_TO_DISAPPEAR
		};


		std::vector<Texture2D> mTextures;

		Rectangle	mSrcRectangle;
		Rectangle	mDstRectangle;
		Color		mColor;

		bool		mInitialized;
		Status      mStatus;
		uint32_t    mFrame;
	};


	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	std::vector<IObjectActivator::OBJECT_ID> registerObjects()
	{
		std::vector<IObjectActivator::OBJECT_ID> ids;
		std::vector<IObjectActivator::TAG_ID>	 tags = { TAG_GAME_OBJECT };

		//////////////////////////////////
		ids.push_back(
			RA_OBJECT_ACTIVATOR.registerObject(
				RA_OBJECT_ACTIVATOR.makeObjectBinder<MyCharacter>(
					&MyCharacter::init, &MyCharacter::fin)
				, tags
			));

		ids.push_back(
			RA_OBJECT_ACTIVATOR.registerObject(
				RA_OBJECT_ACTIVATOR.makeObjectBinder<SoundEffect01>(
					&SoundEffect01::init, &SoundEffect01::fin)
				, tags
			));

		ids.push_back(
			RA_OBJECT_ACTIVATOR.registerObject(
				RA_OBJECT_ACTIVATOR.makeObjectBinder<Music>(
					&Music::init, &Music::fin)
				, tags
			));

		ids.push_back(
			RA_OBJECT_ACTIVATOR.registerObject(
				RA_OBJECT_ACTIVATOR.makeObjectBinder<Cards>(
					&Cards::init, &Cards::fin)
				, tags
			));

		return ids;
	}

}
