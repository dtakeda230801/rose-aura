#pragma once

#include "raylib.h"
#include "RoseAura.h"
#include "Tool.h"

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
	class Sequencer : public ICentralLooper::ITask
		            , public IStoryAnchor::IStoryPointCallback
	{
	public:

		void doTask()
		{
			mObjectActivator.deactivateByTag(TAG_CHILD_OBJECT);
		}

		void onTaskFinish()
		{
		}

		std::string getTaskName()
		{
			return "Sequencer";
		}
		

		void onStateChanged(std::string storyPoint, IStoryAnchor::StoryPointState state)
		{
			Utility::printLog("StoryAnchor onStateChanged: %s %d", storyPoint.c_str(), state);

			if (storyPoint == "Child" && state == IStoryAnchor::StoryPointState::COMPLETED) {
				mCentralLooper.enqueueTask(this);
			}
		}

		void init()
		{
			mStoryAnchor.loadStoryGraph("resources\\story.json");
			mStoryAnchor.registerStoryPointCallback(this);
			mStoryAnchor.changeState("Child", IStoryAnchor::StoryPointState::COMPLETED);
		}

		void fin()
		{
			mStoryAnchor.unregisterStoryPointCallback(this);
		}

		Sequencer(RoseAura& ra) 
			: mCentralLooper(ra.getCentralLooper())
			, mObjectActivator(ra.getObjectActivator())
			, mStoryAnchor(ra.getStoryAnchor())
		{
		}

		virtual ~Sequencer() = default;

	private:
		ICentralLooper&   mCentralLooper;
		IObjectActivator& mObjectActivator;
		IStoryAnchor&     mStoryAnchor;
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
				objectRepository.makeObjectBinder<Sequencer, RoseAura&>(
					&Sequencer::init, &Sequencer::fin, ra)
				, tags
			));

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

		return ids;
	}
}

