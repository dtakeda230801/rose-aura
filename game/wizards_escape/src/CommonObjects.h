#pragma once

#include "raylib.h"
#include "RoseAura.h"
#include "DummyGame.h"

namespace CommonObjects {

	//////////////////////////////////////////////////////////////
	class ContinuousInputTask :
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
			return "ContinuousInputTask";
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

		ContinuousInputTask(RoseAura& ra) :
			mCentralLooper(ra.getCentralLooper())
			, mInputHandler(ra.getInputHandler())
		{
		}

		virtual ~ContinuousInputTask() = default;
	private:
		ICentralLooper& mCentralLooper;
		IInputHandler& mInputHandler;
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
			ClearBackground(BLACK);
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
	class Story : public IStoryAnchor::IStoryPointCallback
	{
	public:
		//////////////////////////////
		class ObjActivatorTask : public ICentralLooper::ITask {
		public:
			void doTask()
			{
				if (mActivate) {
					mObjectActivator.activateByTag(mTag);
				} else {
					mObjectActivator.deactivateByTag(mTag);
				}
			}

			void onTaskFinish()
			{
			}

			std::string getTaskName()
			{
				return mTaskName;
			}

			ObjActivatorTask(RoseAura& ra, std::string name, IObjectActivator::TAG_ID tag, bool activate) :
				  mObjectActivator(ra.getObjectActivator())
				, mTaskName(name)
				, mTag(tag)
				, mActivate(activate)
			{
			}

			virtual ~ObjActivatorTask() = default;
		private:
			IObjectActivator&		 mObjectActivator;
			std::string				 mTaskName;
			IObjectActivator::TAG_ID mTag;
			bool					 mActivate;
		};

		//////////////////////////////
		void onStateChanged(std::string storyPoint, IStoryAnchor::StoryPointState state) {
			Utility::printLog("Change State: %s / %d", storyPoint.c_str(), state);
			if (storyPoint == "Opening" && state == IStoryAnchor::StoryPointState::AVAILABLE) {
				mCentralLooper.enqueueTask(mActivateOpening.get());
			}

			if (storyPoint == "Opening" && state == IStoryAnchor::StoryPointState::COMPLETED) {
				mCentralLooper.enqueueTask(mDeactivateOpening.get());
			}

			if (storyPoint == "Title" && state == IStoryAnchor::StoryPointState::AVAILABLE) {
				mCentralLooper.enqueueTask(mActivateTitle.get());
			}

			if (storyPoint == "Title" && state == IStoryAnchor::StoryPointState::COMPLETED) {
				mCentralLooper.enqueueTask(mDeactivateTitle.get());
			}

			if (storyPoint == "Game" && state == IStoryAnchor::StoryPointState::AVAILABLE) {
				mCentralLooper.enqueueTask(mActivateGame.get());
			}
		}

		void init()
		{
			mStoryAnchor.registerStoryPointCallback(this);
			mStoryAnchor.changeState("Opening", IStoryAnchor::StoryPointState::AVAILABLE);
		}

		void fin()
		{
			mStoryAnchor.unregisterStoryPointCallback(this);
		}

		Story(RoseAura& ra) :
			  mCentralLooper(ra.getCentralLooper())
			, mStoryAnchor(ra.getStoryAnchor())
			, mActivateOpening(std::make_unique<ObjActivatorTask>(ra,"Activate Opening Task", TAG_OPENING_OBJECT,true))
			, mDeactivateOpening(std::make_unique<ObjActivatorTask>(ra, "Deactivate Opening Task", TAG_OPENING_OBJECT, false))
			, mActivateTitle(std::make_unique<ObjActivatorTask>(ra, "Activate Title Task", TAG_TITLE_OBJECT, true))
			, mDeactivateTitle(std::make_unique<ObjActivatorTask>(ra, "Activate Title Task", TAG_TITLE_OBJECT, false))
			, mActivateGame(std::make_unique<ObjActivatorTask>(ra, "Activate Game Task", TAG_GAME_OBJECT, true))
		{
			mStoryAnchor.loadStoryGraph("resources\\story.json");
		}

		virtual ~Story() = default;

	private:
		ICentralLooper& mCentralLooper;
		IStoryAnchor&   mStoryAnchor;

		std::unique_ptr<ObjActivatorTask>	mActivateOpening;
		std::unique_ptr<ObjActivatorTask>	mDeactivateOpening;
		std::unique_ptr<ObjActivatorTask>	mActivateTitle;
		std::unique_ptr<ObjActivatorTask>	mDeactivateTitle;
		std::unique_ptr<ObjActivatorTask>	mActivateGame;
	};


	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	std::vector<IObjectActivator::OBJECT_ID> registerObjects(RoseAura& ra)
	{
		std::vector<IObjectActivator::OBJECT_ID>  ids;
		std::vector<IObjectActivator::TAG_ID>	  tags = { TAG_COMMON_OBJECT };

		IObjectActivator& objectRepository = ra.getObjectActivator();

		//////////////////////////////////
		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<ContinuousInputTask, RoseAura&>(
					&ContinuousInputTask::init, &ContinuousInputTask::fin, ra)
				, tags
			));

		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<Background, RoseAura&>(
					&Background::init, &Background::fin, ra)
				, tags
			));

		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<Story, RoseAura&>(
					&Story::init, &Story::fin, ra)
				, tags
			));

		return ids;
	}
}

