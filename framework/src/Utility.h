#pragma once

#include <vector>

class Utility {
public:

	static void printLog(const char* format, ...);

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


private:

	Utility() {};
	~Utility() {};

};
