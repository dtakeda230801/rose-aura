#pragma once

#include <mutex>

#include "raylib.h"
#include "rlgl.h"

#include "RoseAura.h"
#include "RoseAuraReturnCode.h"
#include "GeometricalUtility.h"
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
		gWorldConf.mWorldSpace.mMin.mX =  100;
		gWorldConf.mWorldSpace.mMin.mY =  100;
		gWorldConf.mWorldSpace.mMin.mZ = -100;
		gWorldConf.mWorldSpace.mMax.mX = WIN_SIZE_W - 100; // 1300
		gWorldConf.mWorldSpace.mMax.mY = WIN_SIZE_H - 100; // 700
		gWorldConf.mWorldSpace.mMax.mZ =  100;

		gWorldConf.mActiveRange.mX = 150;
		gWorldConf.mActiveRange.mY = 100;
		gWorldConf.mActiveRange.mZ = 100;

		gWorldConf.mNonScrollRange.mX = 100;
		gWorldConf.mNonScrollRange.mY = 50;
		gWorldConf.mNonScrollRange.mZ = 50;

		gWorldConf.mPrimaryEntityPosition.mX = 400;
		gWorldConf.mPrimaryEntityPosition.mY = 300;
		gWorldConf.mPrimaryEntityPosition.mZ = 0;

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
			if (!mInitialized) {
				IGraphicsManager::IModelWrapper* modelWrapper = mGraphicsManager.getModelWrapper(mModelId);
				modelWrapper->setAdjustment(3,3,0.37);
				mInitialized = true;
			}
		}

		void render()
		{
			BeginMode3D(mCamera);

			IGraphicsManager::IModelWrapper* modelWrapper = mGraphicsManager.getModelWrapper(mModelId);

			modelWrapper->selectAnimation(0);
			Model* model = static_cast<Model*>(modelWrapper->getAnimetionModel());
			rlDisableBackfaceCulling();
			DrawModel(*model, { 0,0,0 }, 1.0f, WHITE);
			rlEnableBackfaceCulling();

			/*
			rlPushMatrix();

			rlTranslatef(0.0f, 0.0f, 0.0f);
			rlRotatef(mAngle.load(), 0.0f, 1.0f, 0.0f);

			DrawCubeV({ 0, 0, 0 }, { 2, 2, 2 }, RED);
			DrawCubeWiresV({ 0, 0, 0 }, { 2, 2, 2 }, BLACK);

			rlPopMatrix();
			*/
			DrawGrid(10, 1.0f);

			EndMode3D();
		};

		void init()
		{
			mModelId = mGraphicsManager.setModel("main.glb", true);
			mGraphicsManager.setRenderer(this);
			mCentralLooper.registerFrameSyncCallback(this);
			mCentralLooper.enqueueTask(this);
		}

		void fin()
		{
			mGraphicsManager.removeRenderer(this);
			mCentralLooper.unregisterFrameSyncCallback(this);
			mGraphicsManager.releaseModelWrapper(mModelId);
		}

		BackGround3D(RoseAura& ra) :
			  mCentralLooper(ra.getCentralLooper())
			, mGraphicsManager(ra.getGraphicsManager())
			, mAngle(0.0f)
			, mModelId(0)
			, mAnimation(nullptr)
			, mAnimationCount(0)
			, mAnimationFrame(0)
			, mInitialized(false)
		{
			mCamera.position = { 3.0f, 3.0f, 3.0f };
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

		IGraphicsManager::MODEL_ID
						   mModelId;

		ModelAnimation*	   mAnimation;
		int				   mAnimationCount;
		int				   mAnimationFrame;

		bool			   mInitialized;

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
		uint32_t mX = gWorldConf.mPrimaryEntityPosition.mX - gWorldConf.mActiveRange.mX;
		uint32_t mY = gWorldConf.mPrimaryEntityPosition.mY - gWorldConf.mActiveRange.mY;
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
	class MyBall :
		  public IGraphicsManager::IGraphicsRenderer
		, public IInputHandler::IInputHandlerCallback
	{
	public:
		void preprocess()
		{
		}

		void render()
		{
			IWorldNavigator::Vec3 pos = mWorldNavigator.getPrimaryEntityPosition();
			DrawCircle(pos.mX, pos.mY, CIRCLE_SIZE, SKYBLUE);
		};

		void onInputEvent(std::vector<std::pair<InputState, InputType>>& events)
		{
			for (auto event : events) {
				InputState state = event.first;
				InputType  type  = event.second;

				IWorldNavigator::Vec3 pos = mWorldNavigator.getPrimaryEntityPosition();

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
					mWorldNavigator.movePrimaryEntityPosition(pos);
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

		MyBall(RoseAura& ra) :
			  mWorldNavigator(ra.getWorldNavigator())
			, mGraphicsManager(ra.getGraphicsManager())
			, mInputHandler(ra.getInputHandler())
		{
		};

		virtual ~MyBall() = default;

	private:
		IWorldNavigator&  mWorldNavigator;
		IGraphicsManager& mGraphicsManager;
		IInputHandler&    mInputHandler;

		bool       mTextOn = true;
	};

	//////////////////////////////////////////////////////////////
	class MovingBall : public ICentralLooper::ITask
		             , public ICentralLooper::IFrameSyncCallback
		             , public IGraphicsManager::IGraphicsRenderer
		             , public IWorldNavigator::ICollisionCallback
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

			ret = mWorldNavigator.moveEntity(mId, newPos);

			if (RARetCode::RET_OK == ret) {
				mPosition = newPos;
			}
			else if (RARetCode::RET_ADJUSTED == ret) {

				if (RARetCode::RET_OK != mWorldNavigator.getEntityLocation(mId, &mPosition)) {
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
			return "Moving Ball";
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
		CollisionResult onApproaching(IWorldNavigator::WORLD_ID	worldId
			, IWorldNavigator::ENTITY_ID	eventId
			, IWorldNavigator::Vec3 from
			, IWorldNavigator::Vec3& to)
		{
			//Utility::printLog("Approaching...");

			CollisionResult ret = CollisionResult::NO_COLLISION;

			if (CIRCLE_SIZE > GeometricalUtility::calcDistance(mPosition, to)) {
				ret = CollisionResult::HIT;
				mAfterHit = 3;
			}
			return ret;
		};

		void onHit(IWorldNavigator::WORLD_ID	worldId
			     , IWorldNavigator::ENTITY_ID	entityId)
		{
			std::lock_guard<std::mutex> lock(mMutex);
			mColor = YELLOW;
		};

		void init()
		{
			mGraphicsManager.setRenderer(this);
			mWorldNavigator.registerEntity(mId, mPosition, mDistance, this);
			mCentralLooper.registerFrameSyncCallback(this);
			mCentralLooper.enqueueTask(this);
		}

		void fin()
		{
			mCentralLooper.unregisterFrameSyncCallback(this);
			mWorldNavigator.removeEntity(mId);
			mGraphicsManager.removeRenderer(this);
		}

		MovingBall(RoseAura& ra) :
			  mCentralLooper(ra.getCentralLooper())
			, mGraphicsManager(ra.getGraphicsManager())
			, mWorldNavigator(ra.getWorldNavigator())
		{
		};
		virtual ~MovingBall() = default;

	private:

		ICentralLooper&   mCentralLooper;
		IGraphicsManager& mGraphicsManager;
		IWorldNavigator&  mWorldNavigator;

		std::mutex					mMutex;
		IWorldNavigator::ENTITY_ID mId = 1;
		IWorldNavigator::Vec3		mPosition = { 200,200, 0 };
		int							mXDelta   = -MOVE_DELTA;
		int							mYDelta   = -MOVE_DELTA;
		float						mDistance = 30.f;
		Color						mColor    = RED;
		int							mAfterHit = 0;
	};

	//////////////////////////////////////////////////////////////
	class Wall : public IGraphicsManager::IGraphicsRenderer
		       , public IWorldNavigator::ICollisionCallback
	{
	public:
		//IObjectRenderer
		void preprocess()
		{
		}

		void render()
		{
			DrawRectangle(mMin.mX, mMin.mY, mMax.mX - mMin.mX, mMax.mY - mMin.mY, GRAY);
		};

		//ITriggerCallback
		CollisionResult onApproaching(IWorldNavigator::WORLD_ID		worldId
			                        , IWorldNavigator::ENTITY_ID	entityId
  			                        , IWorldNavigator::Vec3 from
			                        , IWorldNavigator::Vec3& to)
		{
			CollisionResult ret = CollisionResult::NO_COLLISION;

			IWorldNavigator::Vec3 intersection;

			if (GeometricalUtility::detectCollision(mVertex, mIndices, from, to, intersection)) {
				GeometricalUtility::adjustPosition(from, intersection, MOVE_DELTA,to);
				ret = CollisionResult::INHIBITED;
			}

			return ret;
		};

		void onHit(IWorldNavigator::WORLD_ID	worldId
			, IWorldNavigator::ENTITY_ID	    entityId)
		{
		};

		void init()
		{
			mGraphicsManager.setRenderer(this);
			mWorldNavigator.registerEntity(mId, mPosition, mDistance, this);
		}

		void fin()
		{
			mWorldNavigator.removeEntity(mId);
			mGraphicsManager.removeRenderer(this);
		}

		Wall(RoseAura& ra) :
			  mGraphicsManager(ra.getGraphicsManager())
			, mWorldNavigator(ra.getWorldNavigator())
		{
		};
		virtual ~Wall() = default;

	private:
		IGraphicsManager& mGraphicsManager;
		IWorldNavigator&  mWorldNavigator;

		IWorldNavigator::ENTITY_ID  mId = 2;

		IWorldNavigator::Vec3		mPosition = { 650
			                                    , 500
			                                    , 0 };
		IWorldNavigator::Vec3		mMin      = { 600
											    , 499
			                                    , -10 };
		IWorldNavigator::Vec3		mMax      = { 700
												, 501
												, 10 };
		float						mDistance = 100.0f;

		std::vector<IWorldNavigator::Vec3> mVertex = 
		{ {600,500, -10}
     	 ,{700,500, -10}
		 ,{700,500,  10}
		 ,{600,500,  10}
		};
		std::vector<uint32_t> mIndices =
		{ 0, 1, 2
		 ,2, 3, 0
		};

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
				objectRepository.makeObjectBinder<Wall, RoseAura&>(
					&Wall::init, &Wall::fin, ra)
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
				objectRepository.makeObjectBinder<MyBall, RoseAura&>(
					&MyBall::init, &MyBall::fin, ra)
				, tags
			));

		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<MovingBall, RoseAura&>(
					&MovingBall::init, &MovingBall::fin, ra)
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
