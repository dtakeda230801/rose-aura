#pragma once

#include "raylib.h"

#include "RoseAura.h"
#include "RoseAuraReturnCode.h"
#include "MediaUtility.h"
#include "Utility.h"

using namespace RoseAuraMediaUtility;
using namespace RoseAuraReturnCode;

#include "DummyGame.h"

#define TXT_POS_X	10
#define TXT_POS_Y	10

#define CIRCLE_SIZE 20

#define MOVE_DELTA  10

namespace GameObjects {

	static const IObjectRepository::TAG_ID TAG_GAME_OBJECT = 0x2;

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
		gWorldConf.mLimitScrolling = true;
	}

	//////////////////////////////////////////////////////////////
	class GameWorld : public IGraphicsManager::IObjectRenderer
	{
	public:
		void doPreprocess()
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
	class ActiveSpace : public IGraphicsManager::IObjectRenderer
		              , public IWorldNavigator::IActiveSpaceCallback
	{
	public:
		void doPreprocess()
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
		unsigned int mX = gWorldConf.mPosition.mX - gWorldConf.mActiveRange.mX;
		unsigned int mY = gWorldConf.mPosition.mY - gWorldConf.mActiveRange.mY;
		unsigned int mW = gWorldConf.mActiveRange.mX * 2;
		unsigned int mH = gWorldConf.mActiveRange.mY * 2;
	};

	//////////////////////////////////////////////////////////////
	class Text01 : public IGraphicsManager::IObjectRenderer
		         , public IInputHandler::IInputHandlerCallback
	{
	public:
		//IObjectRenderer
		void doPreprocess()
		{
		}

		void render()
		{
			std::lock_guard<std::mutex> lock(mMutex);
			if (mDisplay) {
				DrawText("Rose Aura Dummy Game", TXT_POS_X, TXT_POS_Y, 30, DARKGRAY);
			}
		};

		//IInputHandlerCallback
		void onEvent(std::vector<std::pair<InputState, InputType>>& events)
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

		std::mutex		mMutex;
		bool			mDisplay = true;

	};

	//////////////////////////////////////////////////////////////
	class MyCharacter :
		  public IGraphicsManager::IObjectRenderer
		, public IInputHandler::IInputHandlerCallback
	{
	public:
		void doPreprocess()
		{
		}

		void render()
		{
			IWorldNavigator::Vec3 pos = mWorldNavigator.getPosition();
			DrawCircle(pos.mX, pos.mY, CIRCLE_SIZE, SKYBLUE);
		};

		void onEvent(std::vector<std::pair<InputState, InputType>>& events)
		{
			for (auto event : events) {
				//Utility::printLog("MyDot Input(%d / %d)", event.first, event.second);
				InputState state = event.first;
				InputType  type = event.second;

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
		         , public IGraphicsManager::IObjectRenderer
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

		void finish()
		{
		};

		std::string getTaskName()
		{
			return "Test DotTrigger";
		}

		//IFrameSyncCallback
		void sync()
		{
			mCentralLooper.enqueueTask(this);
		}

		//IObjectRenderer
		void doPreprocess()
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
		int							mXDelta = -MOVE_DELTA;
		int							mYDelta = -MOVE_DELTA;
		float						mDistance = 30.f;
		Color						mColor = RED;
		int							mAfterHit = 0;

	};

	//////////////////////////////////////////////////////////////
	class SoundEffect01 : public IInputHandler::IInputHandlerCallback
		                , public ISoundCoordinator::ISoundRenderer
	{
	public:
		//IInputHandlerCallback
		void onEvent(std::vector<std::pair<InputState, InputType>>& events)
		{
			for (auto event : events) {
				InputState state = event.first;
				InputType  type = event.second;

				if (state == InputState::PUSHED && type == InputType::ACTION2) {
					Utility::printLog("Buffer Delay:%d", mSoundCoordinator.getDelayTime());
					mSoundCoordinator.registerRenderer(this);
				}
			}
		}

		void init()
		{
			mInputHandler.registerCallback(this);
			mWaveFileHolder = new WaveFileHolder("test.wav");
		}

		void fin()
		{
			mSoundCoordinator.unregisterRenderer(this);

			delete mWaveFileHolder;
			mInputHandler.unregisterCallback(this);
		}

		RARetCode requestData(unsigned int requestFrameLen, unsigned int* returnFrameLen, ISoundCoordinator::IDataWriter& writer)
		{

			if (requestFrameLen <= mWaveFileHolder->getRemainFrameLen()) {
				*returnFrameLen = requestFrameLen;
			}
			else {
				*returnFrameLen = mWaveFileHolder->getRemainFrameLen();
			}

			writer.write(mWaveFileHolder->getCurrentFramePointer(), *returnFrameLen);

			mWaveFileHolder->moveCurrentFramePointer(*returnFrameLen);

			if (mWaveFileHolder->getRemainFrameLen() == 0) {
				mWaveFileHolder->reset();
				return RARetCode::RET_END_OF_CONTENT;
			}

			return RARetCode::RET_OK;
		}

		void onFinish()
		{
			Utility::printLog("SoundEffect01 onFinish");
		}

		SoundEffect01(RoseAura& ra) :
			  mInputHandler(ra.getInputHandler())
			, mSoundCoordinator(ra.getSoundCoordinator())
			, mWaveFileHolder(nullptr)
		{
		}
		virtual ~SoundEffect01() = default;

	private:
		IInputHandler&     mInputHandler;
		ISoundCoordinator& mSoundCoordinator;
		WaveFileHolder*    mWaveFileHolder;
	};

	//////////////////////////////////////////////////////////////
	class Music : public IInputHandler::IInputHandlerCallback
		        , public ISoundCoordinator::ISoundRenderer
		        , public PreRenderThread
	{
	public:
		//IInputHandlerCallback
		void onEvent(std::vector<std::pair<InputState, InputType>>& events)
		{
			for (auto event : events) {
				InputState state = event.first;
				InputType  type = event.second;

				if (state == InputState::PUSHED && type == InputType::ACTION3) {

					if (!mPlay) {
						startDecodeThread();
						mSoundCoordinator.registerRenderer(this);
					}
					else {
						mPlay = false;
						termDecodeThread();
					}
				}
			}
		}

		void doWork() {
			mNoFinish = mOpusFileHolder->decode();
		}

		void init()
		{
			mInputHandler.registerCallback(this);
			mOpusFileHolder = new OpusFileHolder("Seeker.opus");
		}

		void fin()
		{
			mSoundCoordinator.unregisterRenderer(this);

			termDecodeThread();
			delete mOpusFileHolder;
			mInputHandler.unregisterCallback(this);
		}

		void startDecodeThread()
		{
			mPlay = true;
			mOpusFileHolder->setJumpPoint(6101808, 2602800);
			mNoFinish = mOpusFileHolder->decode();
			start();
		}

		void termDecodeThread()
		{
			mPlay = false;
			mNoFinish = false;
			mOpusFileHolder->reset();
			finish();
		}

		RARetCode requestData(unsigned int requestFrameLen, unsigned int* returnFrameLen, ISoundCoordinator::IDataWriter& writer)
		{
			RARetCode ret = RARetCode::RET_OK;

			*returnFrameLen = 0;

			while (*returnFrameLen < requestFrameLen) {

				float* decoded;
				unsigned int	decodedFrameLen;
				unsigned int    writeFrameLen = 0;

				mOpusFileHolder->getCurrentPointer(decoded, &decodedFrameLen);

				if (decodedFrameLen > 0) {
					if (decodedFrameLen >= requestFrameLen - *returnFrameLen) {
						writeFrameLen = requestFrameLen - *returnFrameLen;
					}
					else {
						writeFrameLen = decodedFrameLen;
					}

					writer.write(decoded, writeFrameLen);

					mOpusFileHolder->moveReadPointer(writeFrameLen);

					*returnFrameLen += writeFrameLen;

				}
				else if (mNoFinish) {
					wakeUp();
				}

				if (!mPlay || (!mNoFinish && decodedFrameLen == 0)) {
					ret = RARetCode::RET_END_OF_CONTENT;
					break;
				}
			}
			return ret;
		}

		void onFinish()
		{
			Utility::printLog("Music onFinish");
		}


		Music(RoseAura& ra) :
			  mInputHandler(ra.getInputHandler())
			, mSoundCoordinator(ra.getSoundCoordinator())
			, mOpusFileHolder(nullptr)
			, mPlay(false)
			, mNoFinish(true)
		{
		}
		virtual ~Music() = default;

	private:
		IInputHandler&     mInputHandler;
		ISoundCoordinator& mSoundCoordinator;
		OpusFileHolder*    mOpusFileHolder;

		bool               mPlay;
		bool               mNoFinish;
	};

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	std::vector<IObjectRepository::OBJECT_ID> registerObjects(RoseAura& ra)
	{
		std::vector<IObjectRepository::OBJECT_ID> ids;
		std::vector<IObjectRepository::TAG_ID>	  tags = { TAG_GAME_OBJECT };

		IObjectRepository& objectRepository = ra.getObjectRepository();

		//////////////////////////////////
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
