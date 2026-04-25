#pragma once

#include "RoseAura.h"
#include "RoseAuraReturnCode.h"
#include "MediaUtility.h"
#include "Utility.h"

#include "DummyGame.h"


using namespace RoseAuraMediaUtility;
using namespace RoseAuraReturnCode;

namespace OpeningObjects {

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	class Movie : public MovieRenderer::IMovieRendererCallback
	{
	public:

		void onVideoFinish()
		{
			Utility::printLog("onVideoFinish");
		}

		void init()
		{
			mMovieRenderer->playMovie();
		}

		void fin()
		{
			mMovieRenderer->stopPlaying();
		}

		Movie(RoseAura& ra) :
			  mMovieRenderer(std::make_unique<MovieRenderer>(ra, "testColor.webm", (WIN_SIZE_W - 1280)/2, (WIN_SIZE_H - 720)/2, this))
			  //mMovieRenderer(std::make_unique<MovieRenderer>(ra, "bluestone2.webm", (WIN_SIZE_W - 1280)/2, (WIN_SIZE_H - 720)/2, this))
			  //mMovieRenderer(std::make_unique<MovieRenderer>(ra, "test.webm"      , (WIN_SIZE_W - 1280)/2, (WIN_SIZE_H - 720)/2, this))
		{
		}

		virtual ~Movie() = default;

	private:
		std::unique_ptr<MovieRenderer>	mMovieRenderer;
	};

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	std::vector<IObjectActivator::OBJECT_ID> registerObjects(RoseAura& ra)
	{
		std::vector<IObjectActivator::OBJECT_ID> ids;
		std::vector<IObjectActivator::TAG_ID>	  tags = { TAG_OPENING_OBJECT };

		IObjectActivator& objectRepository = ra.getObjectRepository();

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

