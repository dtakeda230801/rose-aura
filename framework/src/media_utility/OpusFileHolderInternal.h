#pragma once

#include <cstdint>

namespace RoseAuraMediaUtility {

	class OpusFileHolderInternal {
	public:
		OpusFileHolderInternal(const char* path);
		bool	 decode();
		void	 getCurrentPointer(float*& data, uint32_t* frameLen);
		void	 moveReadPointer(uint32_t frameLen);
		uint32_t getChannels();
		void	 setJumpPoint(uint64_t point, uint64_t to);
		void	 reset();

		virtual ~OpusFileHolderInternal();

	private:
		static constexpr uint32_t BUFF_FRAME_LEN = 960;

		void* mFile;
		uint32_t	mChannels;
		float* mBuffer;
		uint32_t	mFrameLen;
		uint32_t    mReadPointer;
		bool        mNoFinish;
		bool	    mJump;
		uint64_t    mJumpTo;
		uint64_t    mJumpPoint;
		uint64_t    mFrameCounter;
	};
};