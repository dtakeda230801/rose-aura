#pragma once

#include "raylib.h"
#include "RoseAura.h"
#include "Game.h"

namespace CommonObjects {

	//////////////////////////////////////////////////////////////
	class ContinuousInputTask :
		  public ICentralLooper::ITask
		, public ICentralLooper::IFrameSyncCallback {
	public:
		//ITask
		void doTask()
		{
			RA_INPUT_HANDLER.update();
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
			RA_CENTRAL_LOOPER.enqueueTask(this);
		}

		void init()
		{
			RA_CENTRAL_LOOPER.registerFrameSyncCallback(this);
			RA_CENTRAL_LOOPER.enqueueTask(this);
		}

		void fin()
		{
			RA_CENTRAL_LOOPER.unregisterFrameSyncCallback(this);
		}

		ContinuousInputTask() = default;
		virtual ~ContinuousInputTask() = default;
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
			RA_GRAPHICS_MANAGER.setRenderer(this);
		}

		void fin()
		{
			RA_GRAPHICS_MANAGER.removeRenderer(this);
		}

		Background() = default;
		virtual ~Background() = default;
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
					RA_OBJECT_ACTIVATOR.activateByTag(mTag);
				} else {
					RA_OBJECT_ACTIVATOR.deactivateByTag(mTag);
				}
			}

			void onTaskFinish()
			{
			}

			std::string getTaskName()
			{
				return mTaskName;
			}

			ObjActivatorTask(std::string name, IObjectActivator::TAG_ID tag, bool activate)
				: mTaskName(name)
				, mTag(tag)
				, mActivate(activate)
			{
			}

			virtual ~ObjActivatorTask() = default;
		private:
			std::string				 mTaskName;
			IObjectActivator::TAG_ID mTag;
			bool					 mActivate;
		};

		//////////////////////////////
		void onStateChanged(std::string storyPoint, IStoryAnchor::StoryPointState state) {
			Utility::printLog("Change State: %s / %d", storyPoint.c_str(), state);
			if (storyPoint == "Opening" && state == IStoryAnchor::StoryPointState::AVAILABLE) {
				RA_CENTRAL_LOOPER.enqueueTask(mActivateOpening.get());
			}

			if (storyPoint == "Opening" && state == IStoryAnchor::StoryPointState::COMPLETED) {
				RA_CENTRAL_LOOPER.enqueueTask(mDeactivateOpening.get());
			}

			if (storyPoint == "Title" && state == IStoryAnchor::StoryPointState::AVAILABLE) {
				RA_CENTRAL_LOOPER.enqueueTask(mActivateTitle.get());
			}

			if (storyPoint == "Title" && state == IStoryAnchor::StoryPointState::COMPLETED) {
				RA_CENTRAL_LOOPER.enqueueTask(mDeactivateTitle.get());
			}

			if (storyPoint == "Game" && state == IStoryAnchor::StoryPointState::AVAILABLE) {
				RA_CENTRAL_LOOPER.enqueueTask(mActivateGame.get());
			}
		}

		void init()
		{
			RA_STORY_ANCHOR.registerStoryPointCallback(this);
			RA_STORY_ANCHOR.changeState("Opening", IStoryAnchor::StoryPointState::AVAILABLE);
		}

		void fin()
		{
			RA_STORY_ANCHOR.unregisterStoryPointCallback(this);
		}

		Story()
			: mActivateOpening(std::make_unique<ObjActivatorTask>("Activate Opening Task", TAG_OPENING_OBJECT,true))
			, mDeactivateOpening(std::make_unique<ObjActivatorTask>("Deactivate Opening Task", TAG_OPENING_OBJECT, false))
			, mActivateTitle(std::make_unique<ObjActivatorTask>("Activate Title Task", TAG_TITLE_OBJECT, true))
			, mDeactivateTitle(std::make_unique<ObjActivatorTask>("Activate Title Task", TAG_TITLE_OBJECT, false))
			, mActivateGame(std::make_unique<ObjActivatorTask>("Activate Game Task", TAG_GAME_OBJECT, true))
		{
			RA_STORY_ANCHOR.loadStoryGraph("resources\\story.json");
		}

		virtual ~Story() = default;

	private:
		std::unique_ptr<ObjActivatorTask>	mActivateOpening;
		std::unique_ptr<ObjActivatorTask>	mDeactivateOpening;
		std::unique_ptr<ObjActivatorTask>	mActivateTitle;
		std::unique_ptr<ObjActivatorTask>	mDeactivateTitle;
		std::unique_ptr<ObjActivatorTask>	mActivateGame;
	};


	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	std::vector<IObjectActivator::OBJECT_ID> registerObjects()
	{
		std::vector<IObjectActivator::OBJECT_ID>  ids;
		std::vector<IObjectActivator::TAG_ID>	  tags = { TAG_COMMON_OBJECT };

		//////////////////////////////////
		ids.push_back(
			RA_OBJECT_ACTIVATOR.registerObject(
				RA_OBJECT_ACTIVATOR.makeObjectBinder<ContinuousInputTask>(
					&ContinuousInputTask::init, &ContinuousInputTask::fin)
				, tags
			));

		ids.push_back(
			RA_OBJECT_ACTIVATOR.registerObject(
				RA_OBJECT_ACTIVATOR.makeObjectBinder<Background>(
					&Background::init, &Background::fin)
				, tags
			));

		ids.push_back(
			RA_OBJECT_ACTIVATOR.registerObject(
				RA_OBJECT_ACTIVATOR.makeObjectBinder<Story>(
					&Story::init, &Story::fin)
				, tags
			));

		return ids;
	}
}

