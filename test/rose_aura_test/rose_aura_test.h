#pragma once

#include "pch.h"

#define ROSE_AURA_TEST_BEGIN \
		_CrtMemState before, after, diff; \
		_CrtMemCheckpoint(&before)

#define ROSE_AURA_TEST_FIN \
        _CrtMemCheckpoint(&after); \
            if (_CrtMemDifference(&diff, &before, &after)) \
            { _CrtMemDumpStatistics(&diff);_CrtMemDumpAllObjectsSince(&before); \
            FAIL() << "Memory leak detected"; }