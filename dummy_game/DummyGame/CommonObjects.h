#pragma once

#include "raylib.h"

#include "RoseAura.h"
#include "RoseAuraReturnCode.h"

#include "DummyGame.h"

using namespace RoseAuraReturnCode;

namespace CommonObjects {

	//////////////////////////////////////////////////////////////
	class ContinuousInputHandler :
		  public ICentralLooper::ITask
		, public ICentralLooper::IFrameSyncCallback {
	public:
		//ITask
		void doTask()
		{
			mInputHandler.update();
		};

		void onTaskFinish()
		{
		};

		std::string getTaskName()
		{
			return "Test Task";
		}

		//IFrameSyncCallback
		void onFrameSync()
		{
			mCentralLooper.enqueueTask(this);
		}

		void init()
		{
			mCentralLooper.registerFrameSyncCallback(this);
			mCentralLooper.enqueueTask(this);
		}

		void fin()
		{
			mCentralLooper.unregisterFrameSyncCallback(this);
		}

		ContinuousInputHandler(RoseAura& ra) :
			  mCentralLooper(ra.getCentralLooper())
			, mInputHandler(ra.getInputHandler())
		{
		}

		virtual ~ContinuousInputHandler() = default;
	private:
		ICentralLooper& mCentralLooper;
		IInputHandler&  mInputHandler;
	};

	//////////////////////////////////////////////////////////////
	class WorldCtl : public IInputHandler::IInputHandlerCallback
		           , public ICentralLooper::ITask

	{
	public:
		//ITask
		void doTask()
		{
			if (mActivated) {
				mActivated = false;
				mObjectRepository.deactivateByTag(TAG_GAME_OBJECT);
				mObjectRepository.activateByTag(TAG_OPENING_OBJECT);
			}
			else {
				mActivated = true;
				mObjectRepository.deactivateByTag(TAG_OPENING_OBJECT);
				mObjectRepository.activateByTag(TAG_GAME_OBJECT);
			}
		};

		void onTaskFinish()
		{
		};

		std::string getTaskName()
		{
			return "Test Task";
		}

		//IInputHandlerCallback
		void onInputEvent(std::vector<std::pair<InputState, InputType>>& events)
		{
			for (auto event : events) {
				InputState state = event.first;
				InputType  type = event.second;

				if (state == InputState::PUSHED && type == InputType::ACTION4) {
					mCentralLooper.enqueueTask(this);
				}
			}
		}

		void init()
		{
			mInputHandler.registerCallback(this);
		}

		void fin()
		{
			mInputHandler.unregisterCallback(this);
		}

		WorldCtl(RoseAura& ra) :
			  mCentralLooper(ra.getCentralLooper())
			, mInputHandler(ra.getInputHandler())
			, mObjectRepository(ra.getObjectRepository())
			, mActivated(true)
		{
		}
		virtual ~WorldCtl() = default;

	private:
		ICentralLooper&    mCentralLooper;
		IInputHandler&     mInputHandler;
		IObjectActivator& mObjectRepository;
		bool		       mActivated;
	};

	//////////////////////////////////////////////////////////////
	class Background : public IGraphicsManager::IGraphicsRenderer
	{
	public:
		void preprocess()
		{
		}

		void render()
		{
			ClearBackground(RAYWHITE);
		};

		void init()
		{
			mGraphicsManager.setRenderer(this);
		}

		void fin()
		{
			mGraphicsManager.removeRenderer(this);
		}

		Background(RoseAura& ra) :
			mGraphicsManager(ra.getGraphicsManager())
		{
		}

		virtual ~Background() = default;

	private:
		IGraphicsManager& mGraphicsManager;
	};



	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	std::vector<IObjectActivator::OBJECT_ID> registerObjects(RoseAura& ra)
	{
		std::vector<IObjectActivator::OBJECT_ID> ids;
		std::vector<IObjectActivator::TAG_ID>	  tags = { TAG_COMMON_OBJECT };

		IObjectActivator& objectRepository = ra.getObjectRepository();

		//////////////////////////////////
		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<ContinuousInputHandler, RoseAura&>(
					&ContinuousInputHandler::init, &ContinuousInputHandler::fin, ra)
				, tags
			));

		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<WorldCtl, RoseAura&>(
					&WorldCtl::init, &WorldCtl::fin, ra)
				, tags
			));

		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<Background, RoseAura&>(
					&Background::init, &Background::fin, ra)
				, tags
			));

		return ids;
	}
}

