#pragma once

#include <vector>

namespace RoseAuraMediaUtility {

	/////////////////////////////////////////////
	class WaveFileHolder {
	public:
		WaveFileHolder(const char* path);

		float* getCurrentFramePointer();

		unsigned int getFrameLen();

		unsigned int getSamplingRate();

		unsigned int getChannelNum();

		void reset();

	private:
		unsigned int mChannels;
		unsigned int mSamplingRate;
		unsigned int mFrameLen;
		unsigned int mCurrentFrame;
		float*		 mData;

		virtual ~WaveFileHolder() = default;
	};
}