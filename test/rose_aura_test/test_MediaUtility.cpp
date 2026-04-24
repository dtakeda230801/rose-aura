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
        bool noFinish;
        WavWriter* writer;

        ///////////////////////////////////
        writer = new WavWriter();
        writer->Open("..\\..\\test\\rose_aura_test\\testResultSeeker.wav", 48000, 2);

        noFinish = true;

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

        ///////////////////////////////////
        writer = new WavWriter();
        writer->Open("..\\..\\test\\rose_aura_test\\testResultSeeker_withSeek.wav", 48000, 2);
        
        unsigned long frames = 0;

        noFinish = true;

        opus->reset();
        opus->setJumpPoint(6101808, 2602800);

        while (true) {
            float* data;
            unsigned int dataFrameLen;

            if (noFinish) {
                noFinish = opus->decode();
            }

            opus->getCurrentPointer(data, &dataFrameLen);

            if (dataFrameLen > 0) {
                writer->Write(data, dataFrameLen);
                opus->moveReadPointer(dataFrameLen);
                frames += dataFrameLen;
            }
            else if (!noFinish) {
                writer->Close();
                break;
            }

            if (frames >= 180 * 48000) {
                writer->Close();
                break;
            }
        }

        delete writer;
        ///////////////////////////////////
        delete opus;
	}
	ROSE_AURA_TEST_FIN;
}

void writeVideoDataToCSV(const uint8_t* data, uint32_t width, uint32_t height, const std::string& filename)
{
    std::ofstream ofs(filename);
    if (!ofs) return;

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            ofs << static_cast<int32_t>(data[y * width + x ]);
            if (x != width -1){
                ofs << ",";
            }
        }
        ofs << "\n";
    }
    ofs.close();
}

TEST(testVideoFileHolder, BehaviourTest)
{
    ROSE_AURA_TEST_BEGIN;
    {
        float* dummyBuffer = new float[480 * 2];
        unsigned int  videoFrameCount = 0;

        uint32_t dumpFrame = 5;

        std::unique_ptr<VideoFileHolder> vfh = std::make_unique<VideoFileHolder>("..\\..\\test\\rose_aura_test\\testColor.webm");
        //std::unique_ptr<VideoFileHolder> vfh = std::make_unique<VideoFileHolder>("E:\\works\\Dev\\github\\rose-aura\\dummy_game\\DummyGame\\bluestone.webm");

        VideoFileHolder::DecoderReturnCode decRet = VideoFileHolder::DecoderReturnCode::CONTINUE;

        while (decRet != VideoFileHolder::DecoderReturnCode::FINISH) {
            decRet = vfh->decode();

            if (decRet == VideoFileHolder::DecoderReturnCode::VIDEO) {
                bool vFrameRet = true;
                VideoFileHolder::VideoFrame  frame;
                while (vFrameRet) {
                    vFrameRet = vfh->getVideoFrame(frame);
                    if (vFrameRet) {
                        if (dumpFrame == videoFrameCount) {
                            writeVideoDataToCSV(frame.mY, frame.mWidth, frame.mHeight, "..\\..\\test\\rose_aura_test\\testResultY.csv");
                            writeVideoDataToCSV(frame.mU, frame.mWidth/2, frame.mHeight, "..\\..\\test\\rose_aura_test\\testResultU.csv");
                            writeVideoDataToCSV(frame.mV, frame.mWidth/2, frame.mHeight, "..\\..\\test\\rose_aura_test\\testResultV.csv");
                        }

                        vfh->releaseVideoFrame(frame);
                        videoFrameCount++;
                    }
                }
                Utility::printLog("OUT VIDEO FRAME (%d)", videoFrameCount);
            }

            if (decRet == VideoFileHolder::DecoderReturnCode::AUDIO) {
                bool            aFrameRet   = true;
                unsigned int    returnFrameLen;

                while (aFrameRet) {
                    returnFrameLen = 0;
                    aFrameRet = vfh->getAudioFrame(&dummyBuffer, &returnFrameLen, 480);
                }
            }
        }
        delete[] dummyBuffer;
    }
    ROSE_AURA_TEST_FIN;
};


class TestMovieRenderer : 
      public MovieRenderer::IMovieRendererCallback
    , public ICentralLooper::ITask
    , public ICentralLooper::IFrameSyncCallback {
public:

    void doTask()
    {
        mMr->playMovie();
    }

    void onTaskFinish()
    {
    }
    
    std::string getTaskName()
    {
        return "TestMovieRenderer";
    }

    void onFrameSync() {
        ++mCount;

        if (mCount > 100) {
            mRa.getCentralLooper().enqueueTask(this);
        }

    }

    void onVideoFinish() {
        Utility::printLog("onVideoFinish");
    }

    TestMovieRenderer(RoseAura& ra) :
          mRa(ra)
        , mMr(nullptr)
    {
        mRa.getCentralLooper().registerFrameSyncCallback(this);
        mMr = new MovieRenderer(mRa, "..\\..\\test\\rose_aura_test\\testColor.webm", 0, 0, this);
    }
    virtual ~TestMovieRenderer() {
        mRa.getCentralLooper().unregisterFrameSyncCallback(this);
        delete mMr;
    }

private:
    RoseAura&       mRa;
    uint32_t        mCount = 0;
    MovieRenderer*  mMr;
};



TEST(testMovieRenderer, BehaviourTest)
{
    ROSE_AURA_TEST_BEGIN;
    {
        std::unique_ptr<RoseAura> ra = RoseAura::create();

        ICentralLooper&    cl = ra->getCentralLooper();
        IGraphicsManager&  gm = ra->getGraphicsManager();
        ISoundCoordinator& sc = ra->getSoundCoordinator();

        IGraphicsManager::Conf conf;
        conf.mWindowWidth = 1280;
        conf.mWindowHeight = 720;
        conf.mWindowTitle = "testGraphicsManager";
        conf.mFrameRate = 30;

        TestMovieRenderer* tmr = new TestMovieRenderer(*ra);

        cl.start(30);
        sc.start();

        gm.runUntilClosed(conf);

        sc.stop();
        cl.stop();

        delete tmr;
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