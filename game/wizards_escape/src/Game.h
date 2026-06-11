#pragma once

#include "RoseAura.h"

#define WIN_SIZE_W			1400
#define WIN_SIZE_H			800
#define WIN_TITLE			"Rose Aura"
#define VIDEO_FRAME_RATE	24
#define LOOPER_FRAME_RATE   30

static const IObjectActivator::TAG_ID TAG_COMMON_OBJECT	    = 0x1;
static const IObjectActivator::TAG_ID TAG_GAME_OBJECT		= 0x2;
static const IObjectActivator::TAG_ID TAG_TITLE_OBJECT      = 0x3;
static const IObjectActivator::TAG_ID TAG_OPENING_OBJECT	= 0x4;

#define RA_INSTANCE			 FwHolder::getInstance()->get()
#define RA_CENTRAL_LOOPER    FwHolder::getInstance()->get().getCentralLooper()
#define RA_GRAPHICS_MANAGER  FwHolder::getInstance()->get().getGraphicsManager()
#define RA_INPUT_HANDLER     FwHolder::getInstance()->get().getInputHandler()
#define RA_OBJECT_ACTIVATOR  FwHolder::getInstance()->get().getObjectActivator()
#define RA_WORLD_NAVIGATOR   FwHolder::getInstance()->get().getWorldNavigator()
#define RA_SOUND_COORDINATOR FwHolder::getInstance()->get().getSoundCoordinator()
#define RA_CONDITION_SAVER   FwHolder::getInstance()->get().getConditionSaver()
#define RA_STORY_ANCHOR      FwHolder::getInstance()->get().getStoryAnchor()

class FwHolder {
public:

	static void create()
	{
		if (!mInstance) {
			mInstance = new FwHolder();
		}
	}

	static FwHolder* getInstance()
	{
		return mInstance;
	}

	static void destroy()
	{
		if (mInstance) {
			delete mInstance;
		}
	}

	RoseAura& get()
	{
		return *mRoseAura;
	}

private:
	FwHolder()
		: mRoseAura(RoseAura::create())
	{
	};

	virtual ~FwHolder() = default;

	static FwHolder* mInstance;

	std::unique_ptr<RoseAura> mRoseAura;
};