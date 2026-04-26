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
							mCursor = MENU_NUM - 1;
						}
						mSoundSnapshotRenderer->playSound();
					} else if (type == InputType::DOWN) {
						++mCursor;
						if (mCursor >= MENU_NUM) {
							mCursor = 0;
						}
						mSoundSnapshotRenderer->playSound();
					} else if (type == InputType::ACTION1) {
						switch (mCursor) {
						case 0:
							startGame();
							break;
						case 1:
							openSetting();
							break;
						case 2:
							exit();
							break;
						default:
							break;
						}
					}
				}
			}
		}

		void init()
		{
			IGraphicsManager& gm = mRa.getGraphicsManager();
			IInputHandler&    ih = mRa.getInputHandler();

			mSoundSnapshotRenderer = new SoundSnapshotRenderer(mRa, "test.wav");
			gm.setRenderer(this);
			ih.registerCallback(this);
		}

		void fin()
		{
			IGraphicsManager& gm = mRa.getGraphicsManager();
			IInputHandler&    ih = mRa.getInputHandler();

			ih.unregisterCallback(this);
			gm.removeRenderer(this);
			delete mSoundSnapshotRenderer;
		}

		TitleMenu(RoseAura& ra) :
			  mRa(ra)
			, mSoundSnapshotRenderer(nullptr)
		{
		}

		virtual ~TitleMenu() = default;

	private:
		void startGame()
		{
			IStoryAnchor& sa = mRa.getStoryAnchor();
			sa.changeState("Title", IStoryAnchor::StoryPointState::COMPLETED);
		}

		void openSetting()
		{
		}

		void exit()
		{
			IGraphicsManager& gm = mRa.getGraphicsManager();
			gm.exit();
		}

		Color mColor = { 123, 200, 50, 255 };

		RoseAura&				mRa;
		int32_t					mCursor = 0;
		SoundSnapshotRenderer*	mSoundSnapshotRenderer;

		static constexpr uint32_t		MENU_NUM		   = 3;
		static constexpr const char*	TITLE_MENU_START   = "Game Start";
		static constexpr const char*	TITLE_MENU_SETTING = "SETTING";
		static constexpr const char*	TITLE_MENU_EXIT    = "Exit";
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

