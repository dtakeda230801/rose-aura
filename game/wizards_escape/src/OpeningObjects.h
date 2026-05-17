#pragma once

#include "RoseAura.h"
#include "MediaUtility.h"
#include "DummyGame.h"
#include "Utility.h"

using namespace RoseAuraMediaUtility;

namespace OpeningObjects {

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	class Movie : public MovieRenderer::IMovieRendererCallback
		        , public ICentralLooper::ITask
	{
	public:

		void doTask()
		{
			if (mPlaying) {
				mMovieRenderer->stopPlaying();
				mPlaying = false;

				mStoryAnchor.changeState("Opening", IStoryAnchor::StoryPointState::COMPLETED);
			}
		};

		void onTaskFinish()
		{
		};

		std::string getTaskName()
		{
			return "Finish Movie Task";
		};


		void onVideoFinish()
		{
			Utility::printLog("onVideoFinish");
			mCentralLooper.enqueueTask(this);
		}

		void init()
		{
			mMovieRenderer->playMovie();
			mPlaying = true;
		}

		void fin()
		{
			if (mPlaying) {
				mMovieRenderer->stopPlaying();
				mPlaying = false;
			}
		}

		Movie(RoseAura& ra) :
			  mCentralLooper(ra.getCentralLooper())
			, mStoryAnchor(ra.getStoryAnchor())
			, mMovieRenderer(std::make_unique<MovieRenderer>(ra, "resources\\bluestone.webm", (WIN_SIZE_W - 1280)/2, (WIN_SIZE_H - 720)/2, this))
			, mPlaying(false)
		{
		}

		virtual ~Movie() = default;

	private:
		ICentralLooper&					mCentralLooper;
		IStoryAnchor&					mStoryAnchor;
		std::unique_ptr<MovieRenderer>	mMovieRenderer;
		bool							mPlaying;
	};

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	std::vector<IObjectActivator::OBJECT_ID> registerObjects(RoseAura& ra)
	{
		std::vector<IObjectActivator::OBJECT_ID>  ids;
		std::vector<IObjectActivator::TAG_ID>	  tags = { TAG_OPENING_OBJECT };

		IObjectActivator& objectRepository = ra.getObjectActivator();

		//////////////////////////////////
		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<Movie, RoseAura&>(
					&Movie::init, &Movie::fin, ra)
				, tags
			));

		return ids;
	}
}

