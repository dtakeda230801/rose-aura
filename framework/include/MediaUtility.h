#pragma once

#include <vector>
#include <opus/opusfile.h>

namespace RoseAuraMediaUtility {

	/////////////////////////////////////////////
	class WaveFileHolder {
	public:
		WaveFileHolder(const char* path);

		float* getCurrentFramePointer();

		void moveCurrentFramePointer(unsigned int frameLen);

		unsigned int getFrameLen();

		unsigned int getRemainFrameLen();

		unsigned int getSamplingRate();

		unsigned int getChannelNum();

		void reset();

		virtual ~WaveFileHolder();

	private:
		unsigned int mChannels;
		unsigned int mSamplingRate;
		unsigned int mFrameLen;
		unsigned int mCurrentFrame;
		float*		 mData;
	};

	class OpusFileHolder {
	public:
		OpusFileHolder(const char* path);

		bool decode();

		void getCurrentPointer(float*& data, unsigned int* frameLen);

		void moveReadPointer(unsigned int frameLen);

		unsigned int getChannels();

		void reset();

		virtual ~OpusFileHolder();

	private:
		static constexpr unsigned int BUFF_FRAME_LEN = 960;

		OggOpusFile*	mFile;
		unsigned int	mChannels;
		float*			mBuffer;
		unsigned int	mFrameLen;
		unsigned int    mReadPointer;
	};
}