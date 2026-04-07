#include "pch.h"
#include "rose_aura_test.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <filesystem>

#include "sound/SamplingRateConverter.h"
#include "MediaUtility.h"

using namespace RoseAuraMediaUtility;


bool WriteWaveFile16(
    const char* filename,
    const float* data,
    int frames,
    int channels,
    int sampleRate)
{
    std::ofstream file(filename, std::ios::binary);
    if (!file) return false;

    const int bitsPerSample = 16;
    const int blockAlign = channels * bitsPerSample / 8;
    const int byteRate = sampleRate * blockAlign;
    const int dataSize = frames * blockAlign;

    // =====================
    // RIFF Header
    // =====================
    file.write("RIFF", 4);

    uint32_t chunkSize = 36 + dataSize;
    file.write(reinterpret_cast<char*>(&chunkSize), 4);

    file.write("WAVE", 4);

    // =====================
    // fmt chunk
    // =====================
    file.write("fmt ", 4);

    uint32_t subChunk1Size = 16;
    uint16_t audioFormat = 1; // PCM
    uint16_t numChannels = (uint16_t)channels;
    uint32_t sampleRate32 = sampleRate;
    uint16_t blockAlign16 = blockAlign;
    uint16_t bitsPerSample16 = bitsPerSample;

    file.write(reinterpret_cast<char*>(&subChunk1Size), 4);
    file.write(reinterpret_cast<char*>(&audioFormat), 2);
    file.write(reinterpret_cast<char*>(&numChannels), 2);
    file.write(reinterpret_cast<char*>(&sampleRate32), 4);
    file.write(
        reinterpret_cast<const char*>(static_cast<const void*>(&byteRate)),
        sizeof(byteRate));
    file.write(reinterpret_cast<char*>(&blockAlign16), 2);
    file.write(reinterpret_cast<char*>(&bitsPerSample16), 2);

    // =====================
    // data chunk
    // =====================
    file.write("data", 4);

    uint32_t dataSize32 = dataSize;
    file.write(reinterpret_cast<char*>(&dataSize32), 4);

    // =====================
    // float Å® int16 ïœä∑ÇµÇƒèëÇ´çûÇ›
    // =====================
    for (int i = 0; i < frames * channels; ++i)
    {
        float v = std::clamp(data[i], -1.0f, 1.0f);

        int16_t s = static_cast<int16_t>(v * 32767.0f);
        file.write(reinterpret_cast<char*>(&s), sizeof(int16_t));
    }

    return true;
}


TEST(testSamplingRateConverter, APITest)
{
	ROSE_AURA_TEST_BEGIN;
	{
        float*          out;
        unsigned int    outFrameLen;
        WaveFileHolder* waveFileHolder;

        Utility::printLog("path:%s", std::filesystem::current_path().string().c_str());

        waveFileHolder = new WaveFileHolder("..\\..\\test\\rose_aura_test\\test.wav");

		SamplingRateConverter* src = new SamplingRateConverter();

        WriteWaveFile16("..\\..\\test\\rose_aura_test\\testResult1.wav"
                      , waveFileHolder->getCurrentFramePointer()
                      , waveFileHolder->getFrameLen()
                      , waveFileHolder->getChannelNum()
                      , waveFileHolder->getSamplingRate() );

        //////////////////////////////////////////
        waveFileHolder->reset();

        src->setConfig(waveFileHolder->getSamplingRate(), waveFileHolder->getChannelNum(), 44100);

        src->apply(waveFileHolder->getCurrentFramePointer()
                 , waveFileHolder->getFrameLen()
                 , &out
                 , &outFrameLen);

        Utility::printLog("testOut2 : %d samples", outFrameLen);

        WriteWaveFile16("..\\..\\test\\rose_aura_test\\testResult2.wav"
                        , out
                        , outFrameLen
                        , waveFileHolder->getChannelNum()
                        , 44100);

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

        WriteWaveFile16("..\\..\\test\\rose_aura_test\\testResult3.wav"
                       , &couvertOut[0]
                       , convertOutCount / waveFileHolder->getChannelNum()
                       , waveFileHolder->getChannelNum()
                       , 44100);
        Utility::printLog("testOut3 : %d samples", convertOutCount / waveFileHolder->getChannelNum());

        delete src;
        delete waveFileHolder;
	}
	ROSE_AURA_TEST_FIN;
}