#include "pch.h"
#include "rose_aura_test.h"


#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <filesystem>
#include <string>

#include <MediaUtility.h>

#include <Windows.h>
#include <synchapi.h>

using namespace RoseAuraMediaUtility;

class TestPreRenderThread : public PreRenderThread {
public:
    void doWork() {
        mCount++;
    }

    unsigned int getCount() {
        return mCount;
    }

    TestPreRenderThread() :
        mCount(0)
    {
    }
    virtual ~TestPreRenderThread() = default;

private:
    unsigned int   mCount;
};



TEST(testOpusFileHolder, BehaviourTest)
{
	ROSE_AURA_TEST_BEGIN;
	{
        OpusFileHolder* opus   = new OpusFileHolder("..\\..\\test\\rose_aura_test\\Seeker.opus");
        WavWriter* writer      = new WavWriter();
        writer->Open("..\\..\\test\\rose_aura_test\\testResultSeeker.wav",48000,2);

        bool noFinish = true;

        while (true) {
            float*       data;
            unsigned int dataFrameLen;

            if (noFinish) {
                noFinish = opus->decode();
            }

            opus->getCurrentPointer(data, &dataFrameLen);

            if (dataFrameLen > 0) {
                writer->Write(data, dataFrameLen);
                opus->moveReadPointer(dataFrameLen);
            } else if (!noFinish){
                writer->Close();
                break;
            }
        }
        delete writer;
        delete opus;
	}
	ROSE_AURA_TEST_FIN;
}

TEST(testPreRenderThread, BehaviourTest)
{
    ROSE_AURA_TEST_BEGIN;
    {
        bool         ret;
        unsigned int count;

        std::unique_ptr<TestPreRenderThread> preRenderThread = std::make_unique<TestPreRenderThread>();

        ret = preRenderThread->start();
        EXPECT_TRUE(ret);

        ret = preRenderThread->start();
        EXPECT_FALSE(ret);

        count = preRenderThread->getCount();

        EXPECT_EQ(count, 1);

        preRenderThread->wakeUp();

        Sleep(100);

        count = preRenderThread->getCount();

        EXPECT_EQ(count, 2);

        preRenderThread->finish();

    }
    ROSE_AURA_TEST_FIN;
};