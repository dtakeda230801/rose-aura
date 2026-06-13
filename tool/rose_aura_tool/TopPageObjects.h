#pragma once

#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "raylib.h"
#include "RoseAura.h"
#include "Tool.h"
#include "Utility.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace TopPageObjects {

	class ToolList : public ICentralLooper::ITask 
		           , public IGraphicsManager::IGraphicsRenderer
		           , public IInputHandler::IInputHandlerCallback
	{
	public:

		void doTask()
		{
			IObjectActivator& oa = mRa.getObjectActivator();
			IStoryAnchor&     sa = mRa.getStoryAnchor();
			if (RARetCode::RET_OK == oa.activateByTag(mToolList[mCursor].mTag)) {
				sa.changeState("TopPage", IStoryAnchor::StoryPointState::COMPLETED);
				mInputEnable = false;
			}
		}

		void onTaskFinish()
		{
		}

		std::string getTaskName()
		{
			return "TopPageObjects#ToolList";
		}

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
			IObjectActivator& oa = mRa.getObjectActivator();
			bool activatedChild = oa.isActivateByTag(mToolList[mCursor].mTag);

			if (mInitialized && !activatedChild) {
				mInputEnable = true;
				Vector2 pos = { LIST_LEFT,LIST_TOP };

				for (auto& tool : mToolList) {
					DrawTextEx(*mFont, tool.mToolName.c_str(), pos, FONT_SIZE, SPACING, WHITE);
					pos.y += FONT_SIZE;

				}
				float forcusY = FONT_SIZE * mCursor + LIST_TOP + FONT_SIZE;

				DrawLine(static_cast<int32_t>(LIST_LEFT)
					   , static_cast<int32_t>(forcusY)
					   , static_cast<int32_t>(LIST_LEFT + CURSOR_LINE_LEN)
					   , static_cast<int32_t>(forcusY), CURSOR_LINE_COLOR);
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
							mCursor = mToolNum - 1;
						}
					} else if (type == InputType::DOWN) {
						++mCursor;
						if (mCursor >= mToolNum) {
							mCursor = 0;
						}
					} else if (type == InputType::ACTION1) {
						ICentralLooper& cl = mRa.getCentralLooper();
						cl.enqueueTask(this);
						break;
					}
				}
			}
		}

		void init()
		{
			IGraphicsManager& gm = mRa.getGraphicsManager();
			IInputHandler&    ih = mRa.getInputHandler();

			loadToolEntry();

			mFontId = gm.setFont("resources\\NotoSerifJP-Regular.ttf");

			gm.setRenderer(this, IGraphicsManager::Layer::L_FRONT);
			ih.registerCallback(this);
		}

		void fin()
		{
			IGraphicsManager& gm = mRa.getGraphicsManager();
			IInputHandler& ih    = mRa.getInputHandler();

			ih.unregisterCallback(this);
			gm.removeRenderer(this);

			gm.removeFont(mFontId);
		}

		ToolList(RoseAura& ra) :
			  mRa(ra)
			, mCursor(0)
			, mInitialized(false)
			, mToolNum(0)
			, mFontId(0)
			, mFont(nullptr)
			, mInputEnable(false)
		{
		}

		virtual ~ToolList()
		{
		}

	private:
		void loadToolEntry() {
			std::ifstream file("resources\\tool_list.json");

			if (!file) {
				Utility::printLog("can not read tool_list.json");
				return;
			}

			std::stringstream buffer;
			buffer << file.rdbuf();

			json j;
			try {
				j = json::parse(buffer.str());
			}
			catch (const json::parse_error& e) {
				Utility::printLog("json perse error:%s", e.what());
				return;
			}

			for (const auto& tool : j["tools"]) {
				std::string name  = tool["name"];
				uint32_t    tagId = tool["tag_id"];

				mToolList.emplace_back(name,tagId);
			}

			mToolNum = static_cast<int32_t>(mToolList.size());

			return;
		}

		/////////////////////////////////////////////
		struct Tool {
			std::string					mToolName;
			IObjectActivator::TAG_ID	mTag;
		};

		static constexpr float    LIST_TOP   = 50;
		static constexpr float    LIST_LEFT  = 50;

		static constexpr float    FONT_SIZE   = 30.0f;
		static constexpr float    SPACING     = 5.0f;

		static constexpr float    CURSOR_LINE_LEN   = 500.0f;
		static constexpr Color    CURSOR_LINE_COLOR = { 123, 200, 50, 255 };

		std::vector<Tool>		  mToolList;
		int32_t   				  mToolNum;

		RoseAura&				  mRa;
		int32_t					  mCursor;
		bool                      mInitialized;

		IGraphicsManager::FONT_ID mFontId;
		Font*					  mFont;

		bool					  mInputEnable;
	};


	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	std::vector<IObjectActivator::OBJECT_ID> registerObjects(RoseAura& ra)
	{
		std::vector<IObjectActivator::OBJECT_ID>  ids;
		std::vector<IObjectActivator::TAG_ID>	  tags = { TAG_TOP_PAGE_OBJECT };

		IObjectActivator& objectRepository = ra.getObjectActivator();

		//////////////////////////////////
		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<ToolList, RoseAura&>(
					&ToolList::init, &ToolList::fin, ra)
				, tags
			));

		return ids;
	}
}
