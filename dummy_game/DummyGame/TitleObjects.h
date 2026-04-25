#pragma once

#include "raylib.h"

#include "RoseAura.h"
#include "MediaUtility.h"
#include "DummyGame.h"
#include "Utility.h"

using namespace RoseAuraMediaUtility;
using namespace RoseAuraReturnCode;

namespace TitleObjects {

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	class TitleMenu : public IGraphicsManager::IGraphicsRenderer
		            , public IInputHandler::IInputHandlerCallback
		            , public ISoundCoordinator::ISoundRenderer
	{
	public:

		void preprocess()
		{
		}

		void render()
		{
			DrawText(TITLE_MENU_START   , WIN_SIZE_W / 2 - 85, 500, 30, RAYWHITE);
			DrawText(TITLE_MENU_SETTING , WIN_SIZE_W / 2 - 65, 540, 30, RAYWHITE);
			DrawText(TITLE_MENU_EXIT    , WIN_SIZE_W / 2 - 25, 580, 30, RAYWHITE);

			DrawLine(WIN_SIZE_W / 2 - 100, 530 + mCursor * 40, WIN_SIZE_W / 2 + 100, 530 + mCursor * 40, mColor);
		}

		void onInputEvent(std::vector<std::pair<InputState, InputType>>& events)
		{
			for (auto event : events) {
				InputState state = event.first;
				InputType  type = event.second;

				if (state == InputState::PUSHED) {
					if (type == InputType::UP) {
						--mCursor;
						if (mCursor <  0) {
							mCursor = MENU_NUM;
						}
						mSoundCoordinator.registerRenderer(this);
					} else if (type == InputType::DOWN) {
						++mCursor;
						if (mCursor == MENU_NUM) {
							mCursor = 0;
						}
						mSoundCoordinator.registerRenderer(this);
					} else if (type == InputType::ACTION2) {
						switch (mCursor) {
						case 0:
							startGame();
							break;
						case 1:
							openSetting();
							break;
						case 2:
							exit();
						default:
							break;
						}
					}
				}
			}
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

		void onAudioStreamFinish()
		{
			Utility::printLog("TitleMenu onFinish");
		}

		void init()
		{
			mWaveFileHolder = new WaveFileHolder("test.wav");
			mGraphicsManager.setRenderer(this);
			mInputHandler.registerCallback(this);
		}

		void fin()
		{
			mInputHandler.unregisterCallback(this);
			mGraphicsManager.removeRenderer(this);
			delete mWaveFileHolder;
		}

		TitleMenu(RoseAura& ra) :
			  mGraphicsManager(ra.getGraphicsManager())
			, mInputHandler(ra.getInputHandler())
			, mSoundCoordinator(ra.getSoundCoordinator())
			, mStoryAnchor(ra.getStoryAnchor())
			, mWaveFileHolder(nullptr)
		{
		}

		virtual ~TitleMenu() = default;

	private:
		void startGame()
		{
			mStoryAnchor.changeState("Title", IStoryAnchor::StoryPointState::COMPLETED);
		}

		void openSetting()
		{
		}

		void exit()
		{
		}

		IGraphicsManager&	mGraphicsManager;
		IInputHandler&		mInputHandler;
		ISoundCoordinator&  mSoundCoordinator;
		IStoryAnchor&		mStoryAnchor;

		Color mColor = { 123, 200, 50, 255 };

		int32_t			    mCursor            = 0;
		WaveFileHolder*		mWaveFileHolder;

		static constexpr uint32_t MENU_NUM   = 3;

		static constexpr const char* TITLE_MENU_START   = "Game Start";
		static constexpr const char* TITLE_MENU_SETTING = "SETTING";
		static constexpr const char* TITLE_MENU_EXIT    = "Exit";
	};

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	std::vector<IObjectActivator::OBJECT_ID> registerObjects(RoseAura& ra)
	{
		std::vector<IObjectActivator::OBJECT_ID>  ids;
		std::vector<IObjectActivator::TAG_ID>	  tags = { TAG_TITLE_OBJECT };

		IObjectActivator& objectRepository = ra.getObjectActivator();

		//////////////////////////////////
		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<TitleMenu, RoseAura&>(
					&TitleMenu::init, &TitleMenu::fin, ra)
				, tags
			));

		return ids;
	}
}

