#pragma once

#include "raylib.h"

#include "RoseAura.h"
#include "MediaUtility.h"
#include "Game.h"
#include "Utility.h"

using namespace RoseAuraMediaUtility;

namespace TitleObjects {

	class Title : public IGraphicsManager::IGraphicsRenderer
		        , public IInputHandler::IInputHandlerCallback

	{
	public:
		void preprocess()
		{
			if (!mInitialized) {
				mAnimationCoordinator->prepare();
				mInitialized = true;
			}
		}

		void render()
		{
			if (mInitialized) {
				mAnimationCoordinator->countUp();

				mAnimationCoordinator->doAnimation();

				if (mAnimationCoordinator->checkStable()) {
					Font* font = static_cast<Font*>(RA_GRAPHICS_MANAGER.getDefaultFont());

					DrawTextEx(*font, TITLE_MENU_START,    { 100, 540 }, 30, 5, WHITE);
					DrawTextEx(*font, TITLE_MENU_CONTINUE, { 100, 580 }, 30, 5, WHITE);
					DrawTextEx(*font, TITLE_MENU_SETTING,  { 100, 620 }, 30, 5, WHITE);
					DrawTextEx(*font, TITLE_MENU_EXIT,     { 100, 660 }, 30, 5, WHITE);

					DrawLine(100, 570 + mCursor * 40, 300, 570 + mCursor * 40, { 123, 200, 50, 255 });

					if (!mInputEnable) {
						mInputEnable = true;
					}
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
			mAnimationCoordinator->registerElement(new TitleEllement());
			mAnimationCoordinator->registerElement(new CenterEllement());
			mAnimationCoordinator->registerElement(new RingEllement());

			mSoundSnapshotRenderer = new SoundSnapshotRenderer(RA_INSTANCE, "resources\\effect001.wav");

			mFontId = RA_GRAPHICS_MANAGER.setFont("resources\\NotoSerifJP-Regular.ttf");
			RA_GRAPHICS_MANAGER.setRenderer(this);

			RA_INPUT_HANDLER.registerCallback(this);
		}

		void fin()
		{

			RA_GRAPHICS_MANAGER.removeRenderer(this);
			RA_GRAPHICS_MANAGER.removeFont(mFontId);

			RA_INPUT_HANDLER.unregisterCallback(this);

			delete mSoundSnapshotRenderer;

			mAnimationCoordinator->releaseElements();
		}

		Title()
			: mCursor(0)
			, mInitialized(false)
			, mFontId(0)
			, mSoundSnapshotRenderer(nullptr)
			, mInputEnable(false)
			, mAnimationCoordinator(std::make_unique<AnimationCoordinator>(RA_GRAPHICS_MANAGER))
		{
		}

		virtual ~Title() = default;

	private:
		//////////////////////////////////////
		class AnimationCoordinator
		{
		public:

			class AnimationElement
			{
			public:
				uint32_t getStartFrame()
				{
					return mStartFrame;
				}

				virtual void prepare(IGraphicsManager& gm)     = 0;
				virtual void doAnimation(IGraphicsManager& gm) = 0;
				virtual bool isStable() = 0;

				AnimationElement()
					: mRelativeFrame(0)
					, mStartFrame(0)
				{
				}

				virtual ~AnimationElement() = default;
			protected:
				uint32_t	mRelativeFrame;
				uint32_t    mStartFrame;
			};

			void countUp()
			{
				for (auto& elm : mAnimationElms) {
					if (mFrame < elm->getStartFrame()) {
						++mFrame;
						break;
					}
				}
			}

			void prepare() {
				for (auto& elm : mAnimationElms) {
					elm->prepare(mGraphicsManager);
				}
			}

			void doAnimation()
			{
				for (auto& elm : mAnimationElms) {
					if (elm->getStartFrame() <= mFrame) {
						elm->doAnimation(mGraphicsManager);
					}
				}
			}

			bool checkStable()
			{
				bool check = true;
				for (auto& elm : mAnimationElms) {
					if (!elm->isStable()) {
						check = false;
						break;
					}
				}
				return check;
			}

			void registerElement(AnimationElement* elm)
			{
				mAnimationElms.push_back(elm);
			}

			void releaseElements()
			{
				for (auto& elm : mAnimationElms) {
					delete elm;
				}
				mAnimationElms.clear();
			}


			AnimationCoordinator(IGraphicsManager& gm)
				: mFrame(0)
				, mGraphicsManager(gm)
			{
			}
			virtual ~AnimationCoordinator() = default;

		private:
			uint32_t			mFrame;
			IGraphicsManager&	mGraphicsManager;
			std::vector<AnimationElement*>
								mAnimationElms;
		};

		//////////////////////////////////////
		class TitleEllement : public AnimationCoordinator::AnimationElement
		{
		public:

			void prepare(IGraphicsManager& gm)
			{				
				Font* font   = static_cast<Font*>(gm.getDefaultFont());
				Vector2 size = MeasureTextEx(*font, GAME_TITLE, 30, 5);
				mTitlePos.x  = (WIN_SIZE_W - size.x) / 2;
				mTitlePos.y  = (WIN_SIZE_H - size.y) / 2;
			}

			void doAnimation(IGraphicsManager& gm)
			{
				if (!mFinish) {
					Color color = { 255,255,255,255 };

					uint32_t fadeFrame = 72;
					uint32_t dispFrame = 96;

					if (mRelativeFrame < fadeFrame) {
						color.a = static_cast<uint32_t>(255.0 * (float)mRelativeFrame / (float)fadeFrame);
						++mRelativeFrame;
					} else if (mRelativeFrame < fadeFrame + dispFrame  ) {
						++mRelativeFrame;
					} else if (fadeFrame + dispFrame <= mRelativeFrame && mRelativeFrame < dispFrame + fadeFrame * 2) {
						uint32_t frame = mRelativeFrame - (fadeFrame + dispFrame);
						color.a = static_cast<uint32_t>(255.0 * (float)(fadeFrame - frame) / (float)fadeFrame);
						++mRelativeFrame;
					} else if (dispFrame + fadeFrame * 2 <= mRelativeFrame) {
						color.a        = 0;
						mRelativeFrame = 0;
						mFinish        = true;
					}

					Font* font = static_cast<Font*>(gm.getDefaultFont());
					DrawTextEx(*font, GAME_TITLE, mTitlePos, 30, 5, color);

				}
			}

			bool isStable()
			{
				return mFinish;
			}

			TitleEllement()
				: mTitlePos{}
				, mFinish(false)
			{
				mStartFrame = 0;
			}

			virtual ~TitleEllement() = default;
		private:
			Vector2 mTitlePos;
			bool    mFinish;
		};

		//////////////////////////////////////
		class CenterEllement : public AnimationCoordinator::AnimationElement
		{
		public:
			void prepare(IGraphicsManager& gm)
			{
				mTex = LoadTexture("resources\\Title001.png");
				mPos.x = (static_cast<float>(WIN_SIZE_W) - mTex.width) / 2.0f;
				mPos.y = (static_cast<float>(WIN_SIZE_H)- mTex.height) / 2.0f;
			}

			void doAnimation(IGraphicsManager& gm)
			{
				bool ret = false;
				Color color = { 255,255,255,255 };

				uint32_t fadeFrame = 48;

				if (!mFinish && mRelativeFrame < fadeFrame) {
					color.a = static_cast<uint32_t>(255.0 * (float)mRelativeFrame / (float)fadeFrame);
					mRelativeFrame++;
				} else {
					color.a        = 255;
					mRelativeFrame = 0;
					mFinish        = true;
				}

				DrawTexture(mTex, static_cast<int>(mPos.x), static_cast<int>(mPos.y), color);

			}

			bool isStable()
			{
				return mFinish;
			}

			CenterEllement()
				: mTex{}
				, mFinish(false)
				, mPos{}

			{
				mStartFrame = 222;
			}

			virtual ~CenterEllement() = default;

		private:
			Texture2D	mTex;
			bool		mFinish;
			Vector2     mPos;
		};

		class RingEllement : public AnimationCoordinator::AnimationElement
		{
		public:
			void prepare(IGraphicsManager& gm)
			{
				mTex1 = LoadTexture("resources\\Title002.png");
				mTex2 = LoadTexture("resources\\Title003.png");

				mRotationCenter = { mTex1.width / 2.0f, mTex1.height / 2.0f };
				mSrcRectangle   = { 0, 0, (float)mTex1.width, (float)mTex1.height };
				mDstRectangle   = { WIN_SIZE_W / 2.0f , WIN_SIZE_H / 2.0f , (float)mTex1.width, (float)mTex1.height };
			}

			void doAnimation(IGraphicsManager& gm)
			{
				Color color = { 255,255,255,255 };

				uint32_t fadeFrame = 72;

				if (mRelativeFrame < fadeFrame) {
					color.a = static_cast<uint32_t>(255.0 * (float)mRelativeFrame / (float)fadeFrame);
					++mRelativeFrame;
				} else {
					color.a = 255;
					mFinish = true;
				}

				mRotation1 += 1.2f;
				if (mRotation1 == 360.0f) {
					mRotation1 = 0.0f;
				}

				mRotation2 -= 0.5f;
				if (mRotation2 == -360.0f) {
					mRotation2 = 0.0f;
				}

				DrawTexturePro(mTex1, mSrcRectangle, mDstRectangle, mRotationCenter, mRotation1, color);
				DrawTexturePro(mTex2, mSrcRectangle, mDstRectangle, mRotationCenter, mRotation2, color);
			}

			bool isStable()
			{
				return mFinish;
			}

			RingEllement()
				: mTex1{}
				, mTex2{}
				, mRotationCenter{}
				, mSrcRectangle{}
				, mDstRectangle{}
			    , mRotation1(0.0f)
			    , mRotation2(0.0f)
				, mFinish(false)
			{
				mStartFrame = 242;
			}

			virtual ~RingEllement() = default;

		private:
			Texture2D	mTex1;
			Texture2D	mTex2;
			Vector2		mRotationCenter;
			Rectangle	mSrcRectangle;
			Rectangle	mDstRectangle;
			float       mRotation1;
			float       mRotation2;
			bool		mFinish;
		};

		/////////////////////////////////////////////
		void startGame()
		{
			RA_STORY_ANCHOR.changeState("Title", IStoryAnchor::StoryPointState::COMPLETED);
		}

		void selectContinue()
		{
		}

		void openSetting()
		{
		}

		void exit()
		{
			RA_GRAPHICS_MANAGER.exit();
		}

		/////////////////////////////////////////////
		int32_t					  mCursor;
		bool                      mInitialized;
		bool					  mInputEnable;
		IGraphicsManager::FONT_ID mFontId;
		SoundSnapshotRenderer*	  mSoundSnapshotRenderer;
		std::unique_ptr<AnimationCoordinator> 
								  mAnimationCoordinator;

		static constexpr const char* GAME_TITLE = "Wizard's Escape";

		static constexpr uint32_t	 MENU_NUM			 = 4;
		static constexpr const char* TITLE_MENU_START	 = "Game Start";
		static constexpr const char* TITLE_MENU_CONTINUE = "Continue";
		static constexpr const char* TITLE_MENU_SETTING  = "Setting";
		static constexpr const char* TITLE_MENU_EXIT	 = "Exit";

	};

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	std::vector<IObjectActivator::OBJECT_ID> registerObjects()
	{
		std::vector<IObjectActivator::OBJECT_ID>  ids;
		std::vector<IObjectActivator::TAG_ID>	  tags = { TAG_TITLE_OBJECT };

		//////////////////////////////////
		ids.push_back(
			RA_OBJECT_ACTIVATOR.registerObject(
				RA_OBJECT_ACTIVATOR.makeObjectBinder<Title>(
					&Title::init, &Title::fin)
				, tags
			));

		return ids;
	}
}

