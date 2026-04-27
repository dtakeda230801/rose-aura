#pragma once

#include <cstdint>

namespace RoseAuraMediaUtility {

	class WaveFileHolderInternal {
	public:
		WaveFileHolderInternal(const char* path);
		float* getCurrentFramePointer();
		void     moveCurrentFramePointer(uint32_t frameLen);
		uint32_t getFrameLen();
		uint32_t getRemainFrameLen();
		uint32_t getSamplingRate();
		uint32_t getChannelNum();
		void     reset();

		virtual ~WaveFileHolderInternal();

	private:
		uint32_t mChannels;
		uint32_t mSamplingRate;
		uint32_t mFrameLen;
		uint32_t mCurrentFrame;
		float* mData;
	};
};
