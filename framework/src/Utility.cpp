#include "Utility.h"

#include <windows.h>
#include <cstdarg>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <chrono>


#define FLUORITE_LOG_BUFF_SIZE 1024

void Utility::printLog(const char* format, ...) {
    CHAR buffer[FLUORITE_LOG_BUFF_SIZE];

    va_list args;
    va_start(args, format);
    _vsnprintf_s(buffer, FLUORITE_LOG_BUFF_SIZE, _TRUNCATE, format, args);
    va_end(args);

    OutputDebugStringA("[ROSE_AURA DEBUG]");
    OutputDebugStringA(buffer);
    OutputDebugStringA("\n");
}

uint64_t Utility::getCurrentTime()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}
