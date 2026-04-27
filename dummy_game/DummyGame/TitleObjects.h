#pragma once

#include "raylib.h"

#include "RoseAura.h"
#include "MediaUtility.h"
#include "DummyGame.h"
#include "Utility.h"

using namespace RoseAuraMediaUtility;

namespace TitleObjects {

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	class TitleBG001 : public IGraphicsManager::IGraphicsRenderer
	{
	public:
		void preprocess()
		{
			if (!mInitialized) {
				mTex = LoadTexture("Title001.png");
				mInitialized = true;
			}
		}

		void render()
		{
			DrawTexture(mTex, 0, 0, WHITE);
		}

		void init()
		{
			IGraphicsManager& gm = mRa.getGraphicsManager();
			gm.setRenderer(this);
		}

		void fin()
		{
			IGraphicsManager& gm = mRa.getGraphicsManager();
			gm.removeRenderer(this);
		}


		TitleBG001(RoseAura& ra) :
			  mRa(ra)
			, mInitialized(false)
			, mTex{}
		{
		}

		virtual ~TitleBG001() = default;

	private:
		RoseAura& mRa;
		bool      mInitialized;
		Texture2D mTex;

	};

	//////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////
	class TitleBG002 : public ICentralLooper::IFrameSyncCallback
				     , public IGraphicsManager::IGraphicsRenderer
	{
	public:

		void onFrameSync()
		{
			mRotation += 1.2f;
			if (mRotation == 360.0f) {
				mRotation = 0.0f;
			}

		}

		void preprocess()
		{
			if (!mInitialized) {
				mTex            = LoadTexture("Title002.png");
				mRotationCenter = { mTex.width / 2.0f, mTex.height / 2.0f };
				mSrcRectangle   = { 0, 0, (float)mTex.width, (float)mTex.height };
				mDstRectangle   = { mTex.width / 2.0f, mTex.height / 2.0f, (float)mTex.width, (float)mTex.height };
				mInitialized    = true;
			}
		}

		void render()
		{
			DrawTexturePro(mTex, mSrcRectangle, mDstRectangle, mRotationCenter, mRotation, WHITE);
		}

		void init()
		{
			IGraphicsManager& gm = mRa.getGraphicsManager();
			gm.setRenderer(this);

			ICentralLooper& cl = mRa.getCentralLooper();
			cl.registerFrameSyncCallback(this);
		}

		void fin()
		{
			IGraphicsManager& gm = mRa.getGraphicsManager();
			gm.removeRenderer(this);

			ICentralLooper& cl = mRa.getCentralLooper();
			cl.unregisterFrameSyncCallback(this);
		}


		TitleBG002(RoseAura& ra) :
			  mRa(ra)
			, mInitialized(false)
			, mTex{}
			, mRotation(0.0f)
			, mRotationCenter{}
			, mSrcRectangle{}
			, mDstRectangle{}
		{
		}

		virtual ~TitleBG002() = default;

	private:
		RoseAura& mRa;
		bool      mInitialized;
		Texture2D mTex;
		float     mRotation;
		Vector2   mRotationCenter;
		Rectangle mSrcRectangle;
		Rectangle mDstRectangle;

	};

	class TitleBG003 : public ICentralLooper::IFrameSyncCallback
		, public IGraphicsManager::IGraphicsRenderer
	{
	public:

		void onFrameSync()
		{
			mRotation -= 0.5f;
			if (mRotation == -360.0f) {
				mRotation = 0.0f;
			}

		}

		void preprocess()
		{
			if (!mInitialized) {
				mTex = LoadTexture("Title003.png");
				mRotationCenter = { mTex.width / 2.0f, mTex.height / 2.0f };
				mSrcRectangle = { 0, 0, (float)mTex.width, (float)mTex.height };
				mDstRectangle = { mTex.width / 2.0f, mTex.height / 2.0f, (float)mTex.width, (float)mTex.height };
				mInitialized = true;
			}
		}

		void render()
		{
			DrawTexturePro(mTex, mSrcRectangle, mDstRectangle, mRotationCenter, mRotation, WHITE);
		}

		void init()
		{
			IGraphicsManager& gm = mRa.getGraphicsManager();
			gm.setRenderer(this);

			ICentralLooper& cl = mRa.getCentralLooper();
			cl.registerFrameSyncCallback(this);
		}

		void fin()
		{
			IGraphicsManager& gm = mRa.getGraphicsManager();
			gm.removeRenderer(this);

			ICentralLooper& cl = mRa.getCentralLooper();
			cl.unregisterFrameSyncCallback(this);
		}


		TitleBG003(RoseAura& ra) :
			mRa(ra)
			, mInitialized(false)
			, mTex{}
			, mRotation(0.0f)
			, mRotationCenter{}
			, mSrcRectangle{}
			, mDstRectangle{}
		{
		}

		virtual ~TitleBG003() = default;

	private:
		RoseAura& mRa;
		bool      mInitialized;
		Texture2D mTex;
		float     mRotation;
		Vector2   mRotationCenter;
		Rectangle mSrcRectangle;
		Rectangle mDstRectangle;

	};


	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	class TitleMenu : public IGraphicsManager::IGraphicsRenderer
		            , public IInputHandler::IInputHandlerCallback
	{
	public:

		void preprocess()
		{
			if (!mInitialized) {
				IGraphicsManager& gm = mRa.getGraphicsManager();
				mFont = static_cast<Font*>(gm.getFont(mFontId));
				mInitialized = true;
			}
		}

		void render()
		{
			DrawTextEx(*mFont, TITLE_MENU_START,   { 100, 580 }, 30, 5, WHITE);
			DrawTextEx(*mFont, TITLE_MENU_SETTING, { 100, 620 }, 30, 5, WHITE);
			DrawTextEx(*mFont, TITLE_MENU_EXIT,    { 100, 660 }, 30, 5, WHITE);

			DrawLine(100, 610 + mCursor * 40, 300, 610 + mCursor * 40, mColor);
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

			gm.setFont("NotoSerifJP-Regular.ttf", mFontId);

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
			, mFontId(0)
			, mFont(nullptr)
			, mInitialized(false)
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

		RoseAura&				  mRa;
		bool					  mInitialized;
		int32_t					  mCursor = 0;
		SoundSnapshotRenderer*	  mSoundSnapshotRenderer;

		IGraphicsManager::FONT_ID mFontId;
		Font*					  mFont;


		static constexpr uint32_t		MENU_NUM		   = 3;
		static constexpr const char*	TITLE_MENU_START   = "Game Start";
		static constexpr const char*	TITLE_MENU_SETTING = "Setting";
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
				objectRepository.makeObjectBinder<TitleBG003, RoseAura&>(
					&TitleBG003::init, &TitleBG003::fin, ra)
				, tags
			));

		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<TitleBG002, RoseAura&>(
					&TitleBG002::init, &TitleBG002::fin, ra)
				, tags
			));

		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<TitleBG001, RoseAura&>(
					&TitleBG001::init, &TitleBG001::fin, ra)
				, tags
			));

		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<TitleMenu, RoseAura&>(
					&TitleMenu::init, &TitleMenu::fin, ra)
				, tags
			));

		return ids;
	}
}

