#pragma once

#include <gtest/gtest.h>
#include <chrono>
#include <iostream>
#include <fstream>

#include "pch.h"
#include "Utility.h"

#define ROSE_AURA_TEST_BEGIN \
		_CrtMemState before, after, diff; \
		_CrtMemCheckpoint(&before)

#define ROSE_AURA_TEST_FIN \
        _CrtMemCheckpoint(&after); \
            if (_CrtMemDifference(&diff, &before, &after)) \
            { _CrtMemDumpStatistics(&diff);_CrtMemDumpAllObjectsSince(&before); \
            FAIL() << "Memory leak detected"; }

#define ROSE_AURA_MESURMENT_TIME_BEGIN \
        using clock = std::chrono::high_resolution_clock; \
        auto start = clock::now();

#define ROSE_AURA_MESURMENT_TIME_FIN \
        auto end = clock::now(); \
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count(); \
        Utility::printLog("OSE_AURA_MESURMENT_TIME: %lld us",us);

class WavWriter
{
public:
    WavWriter() = default;
    ~WavWriter()
    {
        Close();
    }

    bool Open(const std::string& path,
        uint32_t sampleRate,
        uint16_t channels)
    {
        Close();

        m_sampleRate = sampleRate;
        m_channels = channels;
        m_dataBytes = 0;

        m_file.open(path, std::ios::binary);
        if (!m_file)
            return false;

        WriteHeaderPlaceholder();
        return true;
    }

    bool Write(const float* data, uint32_t frames)
    {
        if (!m_file || !data)
            return false;

        const uint32_t samples = frames * m_channels;

        m_convertBuffer.resize(samples);

        // float Å® int16 ïœä∑
        for (uint32_t i = 0; i < samples; ++i)
        {
            float v = std::clamp(data[i], -1.0f, 1.0f);
            m_convertBuffer[i] = static_cast<int16_t>(v * 32767.0f);
        }

        uint32_t bytes =
            samples * sizeof(int16_t);

        m_file.write(
            reinterpret_cast<const char*>(m_convertBuffer.data()),
            bytes);

        m_dataBytes += bytes;

        return true;
    }

    void Close()
    {
        if (!m_file)
            return;

        FinalizeHeader();
        m_file.close();
    }

private:

    //--------------------------------
    // WAV HEADER
    //--------------------------------

#pragma pack(push,1)
    struct WavHeader
    {
        char     riff[4];        // "RIFF"
        uint32_t chunkSize;
        char     wave[4];        // "WAVE"

        char     fmt[4];         // "fmt "
        uint32_t subchunk1Size;
        uint16_t audioFormat;
        uint16_t numChannels;
        uint32_t sampleRate;
        uint32_t byteRate;
        uint16_t blockAlign;
        uint16_t bitsPerSample;

        char     data[4];        // "data"
        uint32_t dataSize;
    };
#pragma pack(pop)

    void WriteHeaderPlaceholder()
    {
        WavHeader h{};

        memcpy(h.riff, "RIFF", 4);
        memcpy(h.wave, "WAVE", 4);
        memcpy(h.fmt, "fmt ", 4);
        memcpy(h.data, "data", 4);

        h.subchunk1Size = 16;
        h.audioFormat = 1; // PCM
        h.numChannels = m_channels;
        h.sampleRate = m_sampleRate;
        h.bitsPerSample = 16;

        h.blockAlign =
            m_channels * h.bitsPerSample / 8;

        h.byteRate =
            h.sampleRate * h.blockAlign;

        h.dataSize = 0;
        h.chunkSize = 36;

        m_file.write(reinterpret_cast<char*>(&h), sizeof(h));
    }

    void FinalizeHeader()
    {
        WavHeader h{};

        memcpy(h.riff, "RIFF", 4);
        memcpy(h.wave, "WAVE", 4);
        memcpy(h.fmt, "fmt ", 4);
        memcpy(h.data, "data", 4);

        h.subchunk1Size = 16;
        h.audioFormat = 1;
        h.numChannels = m_channels;
        h.sampleRate = m_sampleRate;
        h.bitsPerSample = 16;

        h.blockAlign =
            m_channels * h.bitsPerSample / 8;

        h.byteRate =
            h.sampleRate * h.blockAlign;

        h.dataSize = m_dataBytes;
        h.chunkSize = 36 + m_dataBytes;

        m_file.seekp(0);
        m_file.write(reinterpret_cast<char*>(&h), sizeof(h));
    }

private:
    std::ofstream m_file;

    uint32_t m_sampleRate = 0;
    uint16_t m_channels = 0;
    uint32_t m_dataBytes = 0;

    std::vector<int16_t> m_convertBuffer;
};


