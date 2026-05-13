#pragma once

#include "raylib.h"

#include "RoseAura.h"
#include "MediaUtility.h"
#include "DummyGame.h"
#include "Utility.h"

using namespace RoseAuraMediaUtility;

namespace TitleObjects {

	class Title : public ICentralLooper::IFrameSyncCallback
		        , public IGraphicsManager::IGraphicsRenderer
		        , public IInputHandler::IInputHandlerCallback

	{
	public:
		void onFrameSync()
		{
			if (mCurrentPhase < mPhase.size() - 1) {
				++mFrameCounter;
			}

			mRotation2 += 1.2f;
			if (mRotation2 == 360.0f) {
				mRotation2 = 0.0f;
			}

			mRotation3 -= 0.5f;
			if (mRotation3 == -360.0f) {
				mRotation3 = 0.0f;
			}

		}

		void preprocess()
		{
			if (!mInitialized) {
				IGraphicsManager& gm = mRa.getGraphicsManager();
				mFont = static_cast<Font*>(gm.getFont(mFontId));
				Vector2 size = MeasureTextEx(*mFont, GAME_TITLE, 30, 5);

				mGameTitlePos.x = (WIN_SIZE_W - size.x) / 2;
				mGameTitlePos.y = (WIN_SIZE_H - size.y) / 2;

				mTex1 = LoadTexture("Title001.png");
				mTex2 = LoadTexture("Title002.png");
				mTex3 = LoadTexture("Title003.png");
				mRotationCenter = { mTex1.width / 2.0f, mTex1.height / 2.0f };
				mSrcRectangle = { 0, 0, (float)mTex1.width, (float)mTex1.height };
				mDstRectangle = { mTex1.width / 2.0f, mTex1.height / 2.0f, (float)mTex1.width, (float)mTex1.height };

				mInitialized = true;
			}
		}

		void render()
		{
			bool (Title:: * func)() = mPhase[mCurrentPhase];

			if ((this->*func)()) {
				++mCurrentPhase;
				if (mCurrentPhase == mPhase.size() - 1) {
					mInputEnable = true;
				}
			}
		}

		void onInputEvent(std::vector<std::pair<InputState, InputType>>& events)
		{
			if (!mInputEnable) {
				return;
			}

			for (auto event : events) {
				InputState state = event.first;
				InputType  type = event.second;

				if (state == InputState::PUSHED) {
					if (type == InputType::UP) {
						--mCursor;
						if (mCursor < 0) {
							mCursor = MENU_NUM - 1;
						}
						mSoundSnapshotRenderer->playSound();
					}
					else if (type == InputType::DOWN) {
						++mCursor;
						if (mCursor >= MENU_NUM) {
							mCursor = 0;
						}
						mSoundSnapshotRenderer->playSound();
					}
					else if (type == InputType::ACTION1) {
						switch (mCursor) {
						case 0:
							startGame();
							break;
						case 1:
							selectContinue();
							break;
						case 2:
							openSetting();
							break;
						case 3:
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
			mPhase.push_back(&Title::doPhase1);
			mPhase.push_back(&Title::doPhase2);
			mPhase.push_back(&Title::doPhase3);
			mPhase.push_back(&Title::doPhase4);

			ICentralLooper&   cl = mRa.getCentralLooper();
			IGraphicsManager& gm = mRa.getGraphicsManager();
			IInputHandler&    ih = mRa.getInputHandler();

			mSoundSnapshotRenderer = new SoundSnapshotRenderer(mRa, "test.wav");

			gm.setFont("NotoSerifJP-Regular.ttf", mFontId);
			gm.setRenderer(this);

			ih.registerCallback(this);
			cl.registerFrameSyncCallback(this);
		}

		void fin()
		{
			ICentralLooper& cl   = mRa.getCentralLooper();
			IGraphicsManager& gm = mRa.getGraphicsManager();
			IInputHandler& ih    = mRa.getInputHandler();

			gm.removeRenderer(this);
			gm.removeFont(mFontId);

			cl.unregisterFrameSyncCallback(this);
			ih.unregisterCallback(this);

			delete mSoundSnapshotRenderer;
		}

		Title(RoseAura& ra) :
			  mRa(ra)
			, mCursor(0)
            , mFrameCounter(0)
			, mInitialized(false)
			, mSoundSnapshotRenderer(nullptr)
			, mFontId(0)
			, mFont(nullptr)
			, mGameTitlePos{}
			, mInputEnable(false)
			, mCurrentPhase(0)
			, mTex1{}
			, mTex2{}
			, mTex3{}
			, mRotation2(0.0f)
			, mRotation3(0.0f)
			, mRotationCenter{}
			, mSrcRectangle{}
			, mDstRectangle{}
		{
		}

		virtual ~Title()
		{
		}

	private:

		bool doPhase1()
		{
			bool  ret   = false;
			Color color = {255,255,255,255};

			uint32_t fadeFrame = 72;
			uint32_t dispFrame = 144;


			if (mFrameCounter < fadeFrame) {
				color.a = static_cast<uint32_t>(255.0 * (float)mFrameCounter / (float)fadeFrame);
			} else if (fadeFrame + dispFrame <= mFrameCounter && mFrameCounter < dispFrame + fadeFrame*2) {
				uint32_t frame = mFrameCounter - (fadeFrame + dispFrame);
				color.a = static_cast<uint32_t>(255.0 * (float)(fadeFrame - frame) / (float)fadeFrame);
			} else if (dispFrame + fadeFrame * 2 <= mFrameCounter){
				color.a = 0;
				mFrameCounter = 0;
				ret = true;
			}

			DrawTextEx(*mFont, GAME_TITLE, mGameTitlePos, 30, 5, color);
			return ret;
		}

		bool doPhase2() {
			bool ret = false;
			Color color = { 255,255,255,255 };

			uint32_t fadeFrame = 48;

			if (mFrameCounter < fadeFrame) {
				color.a = static_cast<uint32_t>(255.0 * (float)mFrameCounter / (float)fadeFrame);
			} else {
				color.a = 255;
				mFrameCounter = 0;
				ret = true;
			}

			DrawTexture(mTex1, 0, 0, color);
			return ret;
		}

		bool doPhase3() {

			bool ret = false;
			Color color = { 255,255,255,255 };

			uint32_t fadeFrame = 72;

			if (mFrameCounter < fadeFrame) {
				color.a = static_cast<uint32_t>(255.0 * (float)mFrameCounter / (float)fadeFrame);
			}
			else {
				color.a = 255;
				ret = true;
			}

			DrawTexture(mTex1, 0, 0, WHITE);
			DrawTexturePro(mTex2, mSrcRectangle, mDstRectangle, mRotationCenter, mRotation2, color);
			DrawTexturePro(mTex3, mSrcRectangle, mDstRectangle, mRotationCenter, mRotation3, color);
			return ret;
		}

		bool doPhase4() {

			DrawTexture(mTex1, 0, 0, WHITE);
			DrawTexturePro(mTex2, mSrcRectangle, mDstRectangle, mRotationCenter, mRotation2, WHITE);
			DrawTexturePro(mTex3, mSrcRectangle, mDstRectangle, mRotationCenter, mRotation3, WHITE);

			DrawTextEx(*mFont, TITLE_MENU_START,    { 100, 540 }, 30, 5, WHITE);
			DrawTextEx(*mFont, TITLE_MENU_CONTINUE, { 100, 580 }, 30, 5, WHITE);
			DrawTextEx(*mFont, TITLE_MENU_SETTING,  { 100, 620 }, 30, 5, WHITE);
			DrawTextEx(*mFont, TITLE_MENU_EXIT,     { 100, 660 }, 30, 5, WHITE);

			DrawLine(100, 570 + mCursor * 40, 300, 570 + mCursor * 40, { 123, 200, 50, 255 });

			return false;
		}

		/////////////////////////////////////////////
		void startGame()
		{
			IStoryAnchor& sa = mRa.getStoryAnchor();
			sa.changeState("Title", IStoryAnchor::StoryPointState::COMPLETED);
		}

		void selectContinue()
		{
		}

		void openSetting()
		{
		}

		void exit()
		{
			IGraphicsManager& gm = mRa.getGraphicsManager();
			gm.exit();
		}

		/////////////////////////////////////////////
		RoseAura&				  mRa;
		int32_t					  mCursor;
		uint32_t                  mFrameCounter;
		bool                      mInitialized;

		SoundSnapshotRenderer*	  mSoundSnapshotRenderer;

		IGraphicsManager::FONT_ID mFontId;
		Font*					  mFont;

		Vector2					  mGameTitlePos;

		bool					  mInputEnable;

		static constexpr const char* GAME_TITLE          = "Wizard's Escape";

		static constexpr uint32_t	 MENU_NUM			 = 4;
		static constexpr const char* TITLE_MENU_START	 = "Game Start";
		static constexpr const char* TITLE_MENU_CONTINUE = "Continue";
		static constexpr const char* TITLE_MENU_SETTING  = "Setting";
		static constexpr const char* TITLE_MENU_EXIT	 = "Exit";

		std::vector<bool (Title::*)()> mPhase;
		uint32_t					   mCurrentPhase;

		Texture2D	mTex1;
		Texture2D	mTex2;
		Texture2D	mTex3;

		float     mRotation2;
		float     mRotation3;
		Vector2   mRotationCenter;
		Rectangle mSrcRectangle;
		Rectangle mDstRectangle;
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
				objectRepository.makeObjectBinder<Title, RoseAura&>(
					&Title::init, &Title::fin, ra)
				, tags
			));

		return ids;
	}
}

