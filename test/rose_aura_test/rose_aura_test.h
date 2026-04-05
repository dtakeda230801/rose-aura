#pragma once

#include <gtest/gtest.h>
#include <chrono>
#include <iostream>

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

