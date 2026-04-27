#pragma once

#include <thread>
#include <semaphore>

#include "MediaUtility.h"

namespace RoseAuraMediaUtility {

	class PreRenderThreadInternal {
	public:
		bool start();
		void wakeUp();
		void finish();
		void finishSelf();

		PreRenderThreadInternal(PreRenderThread* parent);
		virtual ~PreRenderThreadInternal() = default;

	private:
		void threadFunc();

		PreRenderThread* mParent;

		std::thread				mThread;
		std::binary_semaphore	mSem;
		std::atomic<bool>		mStarted;
	};
};
