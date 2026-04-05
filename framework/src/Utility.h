#pragma once

#include <vector>

class Utility {
public:

	/////////////////////////////////////////////
	static void printLog(const char* format, ...);

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

	/////////////////////////////////////////////
	class WaveFileHolder {
	public:
		WaveFileHolder(const char* path);

		float* getFramePointer(unsigned int frame);

		unsigned int mChannels;
		unsigned int mSamplingRate;
		unsigned int mFrameLen;
		unsigned int mCurrentFrame;
		std::vector<float> 
					 mSamples;

		virtual ~WaveFileHolder() = default;
	};

private:

	Utility() {};
	~Utility() {};

};
