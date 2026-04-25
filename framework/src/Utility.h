#pragma once

#include <vector>
#include <string>
#include <cstdint>

class Utility {
public:

	/////////////////////////////////////////////
	static void		printLog(const char* format, ...);
	static uint64_t getCurrentTime();

	/////////////////////////////////////////////
	template<class T>
	static int  eraseVectorElm(std::vector<T>& vec, const T& value)
	{
		int ret = 0;

		auto newEnd = std::remove(vec.begin(), vec.end(), value);
		if (newEnd != vec.end()) {
			vec.erase(newEnd, vec.end());
		}
		else {
			ret = -1;
		}

		return ret;
	}

	
};