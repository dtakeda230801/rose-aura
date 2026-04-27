#pragma once

#include "raylib.h"
#include "rlgl.h"

#include "RoseAura.h"
#include "RoseAuraReturnCode.h"
#include "MediaUtility.h"
#include "Utility.h"

#include "DummyGame.h"

using namespace RoseAuraMediaUtility;

#define TXT_POS_X	10
#define TXT_POS_Y	10
#define CIRCLE_SIZE 20
#define MOVE_DELTA  10

namespace GameObjects {

	IWorldNavigator::WorldConfig gWorldConf;

	void buildWorldConf()
	{
		gWorldConf.mWorldSpace.mMin.mX = 100;
		gWorldConf.mWorldSpace.mMin.mY = 100;
		gWorldConf.mWorldSpace.mMin.mZ = 0;
		gWorldConf.mWorldSpace.mMax.mX = WIN_SIZE_W - 100;
		gWorldConf.mWorldSpace.mMax.mY = WIN_SIZE_H - 100;
		gWorldConf.mWorldSpace.mMax.mZ = 0;

		gWorldConf.mActiveRange.mX = 150;
		gWorldConf.mActiveRange.mY = 100;
		gWorldConf.mActiveRange.mZ = 100;

		gWorldConf.mNonScrollRange.mX = 100;
		gWorldConf.mNonScrollRange.mY = 50;
		gWorldConf.mNonScrollRange.mZ = 50;

		gWorldConf.mPosition.mX = 400;
		gWorldConf.mPosition.mY = 300;
		gWorldConf.mPosition.mZ = 0;

		gWorldConf.mEnableFollowing = true;
		gWorldConf.mLimitScrolling  = true;
	}

	//////////////////////////////////////////////////////////////
	class BackGround3D : public IGraphicsManager::IGraphicsRenderer
 		               , public ICentralLooper::ITask
		               , public ICentralLooper::IFrameSyncCallback
	{
	public:
		void doTask()
		{
			float angle = mAngle;

			angle += 5.0f;
			if (angle > 360.0f) {
				angle -= 360;
			}

			mAngle.store(angle);

		};

		void onTaskFinish()
		{
		};

		std::string getTaskName()
		{
			return "BackGround3D";
		}

		void onFrameSync()
		{
			mCentralLooper.enqueueTask(this);
		}

		void preprocess()
		{
		}

		void render()
		{
			BeginMode3D(mCamera);

			rlPushMatrix();

			rlTranslatef(0.0f, 0.0f, 0.0f);
			rlRotatef(mAngle.load(), 0.0f, 1.0f, 0.0f);

			DrawCubeV({ 0, 0, 0 }, { 2, 2, 2 }, RED);
			DrawCubeWiresV({ 0, 0, 0 }, { 2, 2, 2 }, BLACK);

			rlPopMatrix();

			DrawGrid(10, 1.0f);

			EndMode3D();
		};

		void init()
		{
			mGraphicsManager.setRenderer(this);
			mCentralLooper.registerFrameSyncCallback(this);
			mCentralLooper.enqueueTask(this);
		}

		void fin()
		{
			mGraphicsManager.removeRenderer(this);
			mCentralLooper.unregisterFrameSyncCallback(this);
		}

		BackGround3D(RoseAura& ra) :
			  mCentralLooper(ra.getCentralLooper())
			, mGraphicsManager(ra.getGraphicsManager())
			, mAngle(0.0f)
		{
			mCamera.position = { 5.0f, 5.0f, 5.0f };
			mCamera.target   = { 0.0f, 0.0f, 0.0f };
			mCamera.up       = { 0.0f, 1.0f, 0.0f };
			mCamera.fovy     = 45.0f;
			mCamera.projection = CAMERA_PERSPECTIVE; 
		}

		virtual ~BackGround3D() = default;

	private:
		ICentralLooper&    mCentralLooper;
		IGraphicsManager&  mGraphicsManager;
		Camera			   mCamera;
		std::atomic<float> mAngle;
	};

	//////////////////////////////////////////////////////////////
	class GameWorld : public IGraphicsManager::IGraphicsRenderer
	{
	public:
		void preprocess()
		{
		}

		void render()
		{
			Rectangle rect = { static_cast<float>(gWorldConf.mWorldSpace.mMin.mX - CIRCLE_SIZE)
							 , static_cast<float>(gWorldConf.mWorldSpace.mMin.mY - CIRCLE_SIZE)
							 , static_cast<float>(gWorldConf.mWorldSpace.mMax.mX - gWorldConf.mWorldSpace.mMin.mX + (CIRCLE_SIZE*2) )
							 , static_cast<float>(gWorldConf.mWorldSpace.mMax.mY - gWorldConf.mWorldSpace.mMin.mY + (CIRCLE_SIZE*2) ) };
			DrawRectangleLinesEx(rect, 3.0f, LIGHTGRAY);
		};

		void init()
		{
			mGraphicsManager.setRenderer(this);
		}

		void fin()
		{
			mGraphicsManager.removeRenderer(this);
		}

		GameWorld(RoseAura& ra) :
			mGraphicsManager(ra.getGraphicsManager())
		{
		}

		virtual ~GameWorld() = default;

	private:
		IGraphicsManager& mGraphicsManager;
	};

	//////////////////////////////////////////////////////////////
	class ActiveSpace : public IGraphicsManager::IGraphicsRenderer
		              , public IWorldNavigator::IActiveSpaceCallback
	{
	public:
		void preprocess()
		{
		}

		void render()
		{
			std::lock_guard<std::mutex> lock(mMutex);
			Rectangle rect = { static_cast<float>(mX)
							  ,static_cast<float>(mY)
							  ,static_cast<float>(mW)
							  ,static_cast<float>(mH) };
			DrawRectangleLinesEx(rect, 3.0f, BLUE);
		};

		void onUpdate(IWorldNavigator::WORLD_ID worldId, IWorldNavigator::Bounds activeSpace)
		{
			std::lock_guard<std::mutex> lock(mMutex);
			mX = activeSpace.mMin.mX;
			mY = activeSpace.mMin.mY;
			mW = activeSpace.mMax.mX - activeSpace.mMin.mX;
			mH = activeSpace.mMax.mY - activeSpace.mMin.mY;
		}

		void init()
		{
			mWorldNavigator.registerActiveSpaceCallback(this);
			mGraphicsManager.setRenderer(this);
		}

		void fin()
		{
			mGraphicsManager.removeRenderer(this);
			mWorldNavigator.unregisterActiveSpaceCallback();
		}

		ActiveSpace(RoseAura& ra) :
			  mGraphicsManager(ra.getGraphicsManager())
			, mWorldNavigator(ra.getWorldNavigator())
		{

		}

		virtual ~ActiveSpace() = default;

	private:
		IGraphicsManager& mGraphicsManager;
		IWorldNavigator&  mWorldNavigator;

		std::mutex mMutex;
		uint32_t mX = gWorldConf.mPosition.mX - gWorldConf.mActiveRange.mX;
		uint32_t mY = gWorldConf.mPosition.mY - gWorldConf.mActiveRange.mY;
		uint32_t mW = gWorldConf.mActiveRange.mX * 2;
		uint32_t mH = gWorldConf.mActiveRange.mY * 2;
	};

	//////////////////////////////////////////////////////////////
	class Text01 : public IGraphicsManager::IGraphicsRenderer
		         , public IInputHandler::IInputHandlerCallback
	{
	public:
		//IObjectRenderer
		void preprocess()
		{
		}

		void render()
		{
			std::lock_guard<std::mutex> lock(mMutex);
			if (mDisplay) {
				DrawText("Rose Aura Dummy Game", TXT_POS_X, TXT_POS_Y, 30, RAYWHITE);
			}
		};

		//IInputHandlerCallback
		void onInputEvent(std::vector<std::pair<InputState, InputType>>& events)
		{
			for (auto event : events) {
				InputState state = event.first;
				InputType  type = event.second;

				if (state == InputState::PUSHED && type == InputType::ACTION1) {
					if (mDisplay) {
						mDisplay = false;
					}
					else {
						mDisplay = true;
					}
				}
			}
		}

		void init()
		{
			mInputHandler.registerCallback(this);
			mGraphicsManager.setRenderer(this);
		}

		void fin()
		{
			mGraphicsManager.removeRenderer(this);
			mInputHandler.unregisterCallback(this);
		}

		Text01(RoseAura& ra) :
			  mGraphicsManager(ra.getGraphicsManager())
			, mInputHandler(ra.getInputHandler())
		{
		}
		virtual ~Text01() = default;

	private:
		IGraphicsManager& mGraphicsManager;
		IInputHandler&    mInputHandler;

		std::mutex		  mMutex;
		bool			  mDisplay = true;

	};

	//////////////////////////////////////////////////////////////
	class MyCharacter :
		  public IGraphicsManager::IGraphicsRenderer
		, public IInputHandler::IInputHandlerCallback
	{
	public:
		void preprocess()
		{
		}

		void render()
		{
			IWorldNavigator::Vec3 pos = mWorldNavigator.getPosition();
			DrawCircle(pos.mX, pos.mY, CIRCLE_SIZE, SKYBLUE);
		};

		void onInputEvent(std::vector<std::pair<InputState, InputType>>& events)
		{
			for (auto event : events) {
				InputState state = event.first;
				InputType  type  = event.second;

				IWorldNavigator::Vec3 pos = mWorldNavigator.getPosition();

				if (state == InputState::PUSHED || state == InputState::PRESSED) {
					if (type == InputType::UP) {
						pos.mY -= MOVE_DELTA;
					}
					else if (type == InputType::DOWN) {
						pos.mY += MOVE_DELTA;
					}
					else if (type == InputType::LEFT) {
						pos.mX -= MOVE_DELTA;
					}
					else if (type == InputType::RIGHT) {
						pos.mX += MOVE_DELTA;
					}
					mWorldNavigator.movePosition(pos);
				}
			}
		}

		void init()
		{
			mGraphicsManager.setRenderer(this);
			mInputHandler.registerCallback(this);
		}

		void fin()
		{
			mInputHandler.unregisterCallback(this);
			mGraphicsManager.removeRenderer(this);
		}

		MyCharacter(RoseAura& ra) :
			  mWorldNavigator(ra.getWorldNavigator())
			, mGraphicsManager(ra.getGraphicsManager())
			, mInputHandler(ra.getInputHandler())
		{
		};

		virtual ~MyCharacter() = default;

	private:
		IWorldNavigator&  mWorldNavigator;
		IGraphicsManager& mGraphicsManager;
		IInputHandler&    mInputHandler;

		bool       mTextOn = true;
	};

	//////////////////////////////////////////////////////////////
	class Target : public ICentralLooper::ITask
		         , public ICentralLooper::IFrameSyncCallback
		         , public IGraphicsManager::IGraphicsRenderer
		         , public IWorldNavigator::ITriggerCallback
	{
	public:
		//ITask
		void doTask()
		{
			RARetCode ret;

			if (0 < mAfterHit) {
				if (--mAfterHit == 0) {
					mColor = RED;
				}
			}

			IWorldNavigator::Vec3 newPos = { mPosition.mX + mXDelta
										   , mPosition.mY + mYDelta
										   , mPosition.mZ };

			ret = mWorldNavigator.moveTrigger(mId, newPos);

			if (RARetCode::RET_OK == ret) {
				mPosition = newPos;
			}
			else if (RARetCode::RET_ADJUSTED == ret) {

				if (RARetCode::RET_OK != mWorldNavigator.getTriggerLocation(mId, &mPosition)) {
					Utility::printLog("getTriggerLocation fails");
					return;
				}

				if (mPosition.mX != newPos.mX) {
					mXDelta *= -1;
				}

				if (mPosition.mY != newPos.mY) {
					mYDelta *= -1;
				}

			}
			else {
				Utility::printLog("moveTrigger fails");
			}
		};

		void onTaskFinish()
		{
		};

		std::string getTaskName()
		{
			return "Target";
		}

		//IFrameSyncCallback
		void onFrameSync()
		{
			mCentralLooper.enqueueTask(this);
		}

		//IObjectRenderer
		void preprocess()
		{
		}

		void render()
		{
			std::lock_guard<std::mutex> lock(mMutex);
			DrawCircle(mPosition.mX, mPosition.mY, CIRCLE_SIZE, mColor);
		};

		//ITriggerCallback
		bool onApproaching(IWorldNavigator::WORLD_ID	worldId
			, IWorldNavigator::TRIGGER_ID	eventId
			, IWorldNavigator::Vec3& trigerLocation
			, IWorldNavigator::Vec3& position)
		{
			//Utility::printLog("Approaching...");

			bool ret = false;
			if (CIRCLE_SIZE > calcDistance(trigerLocation, position)) {
				ret = true;
				mAfterHit = 3;
			}
			return ret;
		};

		void onTrigger(IWorldNavigator::WORLD_ID	worldId
			, IWorldNavigator::TRIGGER_ID	eventId)
		{
			std::lock_guard<std::mutex> lock(mMutex);
			mColor = YELLOW;
		};

		void init()
		{
			mGraphicsManager.setRenderer(this);
			mWorldNavigator.registerTrigger(mId, mPosition, mDistance, this);
			mCentralLooper.registerFrameSyncCallback(this);
			mCentralLooper.enqueueTask(this);
		}

		void fin()
		{
			mCentralLooper.unregisterFrameSyncCallback(this);
			mWorldNavigator.removeTrigger(mId);
			mGraphicsManager.removeRenderer(this);
		}



		Target(RoseAura& ra) :
			  mCentralLooper(ra.getCentralLooper())
			, mGraphicsManager(ra.getGraphicsManager())
			, mWorldNavigator(ra.getWorldNavigator())
		{
		};
		virtual ~Target() = default;

	private:
		float calcDistance(IWorldNavigator::Vec3& a, IWorldNavigator::Vec3& b)
		{
			return static_cast<float>(std::sqrt(
				(static_cast<double>(b.mX - a.mX) * static_cast<double>(b.mX - a.mX))
				+ (static_cast<double>(b.mY - a.mY) * static_cast<double>(b.mY - a.mY))
				+ (static_cast<double>(b.mZ - a.mZ) * static_cast<double>(b.mZ - a.mZ))));
		}

		ICentralLooper&   mCentralLooper;
		IGraphicsManager& mGraphicsManager;
		IWorldNavigator&  mWorldNavigator;

		std::mutex					mMutex;
		IWorldNavigator::TRIGGER_ID mId = 1;
		IWorldNavigator::Vec3		mPosition = { 200,200, 0 };
		int							mXDelta   = -MOVE_DELTA;
		int							mYDelta   = -MOVE_DELTA;
		float						mDistance = 30.f;
		Color						mColor    = RED;
		int							mAfterHit = 0;

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
			mSoundSnapshotRenderer = new SoundSnapshotRenderer(mRa,"test.wav");
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
			mMusicRenderer = new MusicRenderer(mRa, "Seeker.opus");
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
				objectRepository.makeObjectBinder<BackGround3D, RoseAura&>(
					&BackGround3D::init, &BackGround3D::fin, ra)
				, tags
			));

		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<GameWorld, RoseAura&>(
					&GameWorld::init, &GameWorld::fin, ra)
				, tags
			));

		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<ActiveSpace, RoseAura&>(
					&ActiveSpace::init, &ActiveSpace::fin, ra)
				, tags
			));

		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<Text01, RoseAura&>(
					&Text01::init, &Text01::fin, ra)
				, tags
			));

		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<MyCharacter, RoseAura&>(
					&MyCharacter::init, &MyCharacter::fin, ra)
				, tags
			));

		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<Target, RoseAura&>(
					&Target::init, &Target::fin, ra)
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
