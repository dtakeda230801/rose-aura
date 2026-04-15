#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <semaphore>

namespace RoseAuraMediaUtility {

	/////////////////////////////////////////////
	class WaveFileHolder {
	public:
		WaveFileHolder(const char* path);
		float*   getCurrentFramePointer();
		void     moveCurrentFramePointer(uint32_t frameLen);
		uint32_t getFrameLen();
		uint32_t getRemainFrameLen();
		uint32_t getSamplingRate();
		uint32_t getChannelNum();
		void     reset();

		virtual ~WaveFileHolder();

	private:
		uint32_t mChannels;
		uint32_t mSamplingRate;
		uint32_t mFrameLen;
		uint32_t mCurrentFrame;
		float*	 mData;
	};

	/////////////////////////////////////////////
	class OpusFileHolder {
	public:
		OpusFileHolder(const char* path);
		bool	 decode();
		void	 getCurrentPointer(float*& data, uint32_t* frameLen);
		void	 moveReadPointer(uint32_t frameLen);
		uint32_t getChannels();
		void	 setJumpPoint(uint64_t point, uint64_t to);
		void	 reset();

		virtual ~OpusFileHolder();

	private:
		static constexpr uint32_t BUFF_FRAME_LEN = 960;

		void*		mFile;
		uint32_t	mChannels;
		float*		mBuffer;
		uint32_t	mFrameLen;
		uint32_t    mReadPointer;
		bool        mNoFinish;
		bool	    mJump;
		uint64_t    mJumpTo;
		uint64_t    mJumpPoint;
		uint64_t    mFrameCounter;
	};

	/////////////////////////////////////////////
	class VideoFileHolder {
	public:

		struct VideoFrame {
			uint32_t mWidth;
			uint32_t mHeight;

			uint8_t* mY;
			uint8_t* mU;
			uint8_t* mV;

			uint32_t mStrideY;
			uint32_t mStrideU;
			uint32_t mStrideV;

			uint64_t mTimestamp;
		};

		enum class DecoderReturnCode {
			  AUDIO
			, VIDEO
			, CONTINUE
			, FINISH
		};

		VideoFileHolder(const char* path);

		DecoderReturnCode decode();

		bool     getAudioFrame(float** buff, uint32_t* returnFrameLen, uint32_t requestFrameLen);
		bool     getVideoFrame(VideoFrame& videoFrame);

		void     releaseVideoFrame(VideoFrame& frame);

		uint32_t getSamplingRate();
		uint32_t getChannels();

		virtual ~VideoFileHolder() = default;
	private:
		class VideoFileHolderImpl;
		std::unique_ptr<VideoFileHolderImpl> mImpl;
	};


	/////////////////////////////////////////////
	class PreRenderThread {
	public:
		bool start();
		void wakeUp();
		void finish();
		void finishSelf();

		virtual void doWork() = 0;

		PreRenderThread();
		virtual ~PreRenderThread() = default;
	
	private:
		void threadFunc();

		std::thread				mThread;
		std::binary_semaphore	mSem;
		std::atomic<bool>		mStarted;
	};

	/////////////////////////////////////////////
	class MultiBlockBuffer {
	public:
		MultiBlockBuffer(uint32_t numberOfBlocks, uint32_t framePerBlock, uint32_t elmPerFrame);

		void     getWriteBuffer(float*& buffer, uint32_t& aveilFrameLen);
		void     getReadBuffer(float*& buffer, uint32_t& aveilFrameLen);
		bool     updateWriteBuffer(uint32_t writeFrameLen, uint64_t* attribute);
		bool     updateReadBuffer(uint32_t readFrameLen, uint64_t* attribute);
		uint32_t getAvailBlockNum();


		virtual ~MultiBlockBuffer();
	private:
		void* mImpl;
	};
}