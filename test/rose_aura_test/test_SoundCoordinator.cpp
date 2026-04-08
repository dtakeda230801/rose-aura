#include "pch.h"
#include "rose_aura_test.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <filesystem>

#include <RoseAura.h>
#include "MediaUtility.h"
#include "sound/SamplingRateConverter.h"

using namespace RoseAuraMediaUtility;

TEST(testSoundCoordinator, APITest)
{
    ROSE_AURA_TEST_BEGIN;
    {
        std::unique_ptr<RoseAura> ra = RoseAura::create();

        ISoundCoordinator& sc = ra->getSoundCoordinator();

    }
    ROSE_AURA_TEST_FIN;
}

TEST(testSamplingRateConverter, BehaviourTest)
{
	ROSE_AURA_TEST_BEGIN;
	{
        float*                  out;
        unsigned int            outFrameLen;
        WaveFileHolder*         waveFileHolder;
        WavWriter*              wavWriter;
        SamplingRateConverter*  src;


        wavWriter = new WavWriter();
        src       = new SamplingRateConverter();

        Utility::printLog("path:%s", std::filesystem::current_path().string().c_str());

        waveFileHolder = new WaveFileHolder("..\\..\\test\\rose_aura_test\\test.wav");

        wavWriter->Open("..\\..\\test\\rose_aura_test\\testResult1.wav"
                      , waveFileHolder->getSamplingRate()
                      , static_cast<unsigned short>(waveFileHolder->getChannelNum()));

        wavWriter->Write(waveFileHolder->getCurrentFramePointer(), waveFileHolder->getFrameLen());

        wavWriter->Close();

        //////////////////////////////////////////
        waveFileHolder->reset();

        src->setConfig(waveFileHolder->getSamplingRate(), waveFileHolder->getChannelNum(), 44100);

        src->apply(waveFileHolder->getCurrentFramePointer()
                 , waveFileHolder->getFrameLen()
                 , &out
                 , &outFrameLen);

        Utility::printLog("testOut2 : %d samples", outFrameLen);

        wavWriter->Open("..\\..\\test\\rose_aura_test\\testResult2.wav"
            , 44100
            , static_cast<unsigned short>(waveFileHolder->getChannelNum()));

        wavWriter->Write(out, outFrameLen);

        wavWriter->Close();

        src->releaseBuffer();

        //////////////////////////////////////////
        std::vector<float>  couvertOut;
        unsigned int        convertOutCount = 0;

        unsigned int        convertWinSize = 500;

        waveFileHolder->reset();

        src->reset();
        src->setConfig(waveFileHolder->getSamplingRate(), waveFileHolder->getChannelNum(), 44100);

        ROSE_AURA_MESURMENT_TIME_BEGIN;
        while (true) {
            int win;

            if (convertWinSize < waveFileHolder->getRemainFrameLen()) {
                win = convertWinSize;
            }
            else {
                win = waveFileHolder->getRemainFrameLen();
            }

            src->apply(waveFileHolder->getCurrentFramePointer(), win, &out, &outFrameLen);
            waveFileHolder->moveCurrentFramePointer(win);
            couvertOut.resize(couvertOut.size() + (outFrameLen * waveFileHolder->getChannelNum()));
            for (unsigned int i = 0; i < outFrameLen * waveFileHolder->getChannelNum(); i++) {
                couvertOut[convertOutCount + i] = *out++;
            }
            convertOutCount += outFrameLen * waveFileHolder->getChannelNum();

            src->releaseBuffer();

            if (waveFileHolder->getRemainFrameLen() == 0) {
                break;
            }

        }
        ROSE_AURA_MESURMENT_TIME_FIN;

        wavWriter->Open("..\\..\\test\\rose_aura_test\\testResult3.wav"
            , 44100
            , static_cast<unsigned short>(waveFileHolder->getChannelNum()));

        wavWriter->Write(&couvertOut[0], convertOutCount / waveFileHolder->getChannelNum());

        wavWriter->Close();

        Utility::printLog("testOut3 : %d samples", convertOutCount / waveFileHolder->getChannelNum());

        delete src;
        delete wavWriter;
        delete waveFileHolder;
	}
	ROSE_AURA_TEST_FIN;
}