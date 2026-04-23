#include <windows.h>
#include <processthreadsapi.h>
#include <WinBase.h>

#include "MediaUtility.h"
#include "PreRenderThreadInternal.h"

using namespace RoseAuraMediaUtility;

bool PreRenderThreadInternal::start()
{
    if (mStarted.load(std::memory_order_acquire)) {
        return false;
    }
    mThread = std::thread(&PreRenderThreadInternal::threadFunc, this);
    mStarted.wait(false);
    return true;
}

void PreRenderThreadInternal::wakeUp()
{
    mSem.release();
}

void PreRenderThreadInternal::finish()
{
    mStarted.store(false, std::memory_order_release);
    mSem.release();

    if (mThread.joinable()) {
        mThread.join();
    }
}

void PreRenderThreadInternal::finishSelf()
{
    mStarted.store(false, std::memory_order_release);
}

PreRenderThreadInternal::PreRenderThreadInternal(PreRenderThread* parent) :
      mParent(parent)
    , mStarted(false)
    , mSem(0)
{
}

void PreRenderThreadInternal::threadFunc()
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);

    mStarted.store(true, std::memory_order_release);
    mStarted.notify_one();

    while (mStarted.load(std::memory_order_acquire)) {
        mParent->doWork();
        mSem.acquire();
    }
}
