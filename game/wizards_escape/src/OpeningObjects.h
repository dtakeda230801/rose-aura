#pragma once

#include "RoseAura.h"
#include "MediaUtility.h"
#include "Game.h"
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

				RA_STORY_ANCHOR.changeState("Opening", IStoryAnchor::StoryPointState::COMPLETED);
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
			RA_CENTRAL_LOOPER.enqueueTask(this);
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

		Movie() 
			: mMovieRenderer(std::make_unique<MovieRenderer>(RA_INSTANCE, "resources\\bluestone.webm", (WIN_SIZE_W - 1280)/2, (WIN_SIZE_H - 720)/2, this))
			, mPlaying(false)
		{
		}

		virtual ~Movie() = default;

	private:
		std::unique_ptr<MovieRenderer>	mMovieRenderer;
		bool							mPlaying;
	};

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	std::vector<IObjectActivator::OBJECT_ID> registerObjects()
	{
		std::vector<IObjectActivator::OBJECT_ID>  ids;
		std::vector<IObjectActivator::TAG_ID>	  tags = { TAG_OPENING_OBJECT };

		//////////////////////////////////
		ids.push_back(
			RA_OBJECT_ACTIVATOR.registerObject(
				RA_OBJECT_ACTIVATOR.makeObjectBinder<Movie>(
					&Movie::init, &Movie::fin)
				, tags
			));

		return ids;
	}
}

