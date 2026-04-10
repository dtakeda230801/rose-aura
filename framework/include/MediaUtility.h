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
		float*       getCurrentFramePointer();
		void         moveCurrentFramePointer(unsigned int frameLen);
		unsigned int getFrameLen();
		unsigned int getRemainFrameLen();
		unsigned int getSamplingRate();
		unsigned int getChannelNum();
		void         reset();

		virtual ~WaveFileHolder();

	private:
		unsigned int mChannels;
		unsigned int mSamplingRate;
		unsigned int mFrameLen;
		unsigned int mCurrentFrame;
		float*		 mData;
	};

	/////////////////////////////////////////////
	class OpusFileHolder {
	public:
		OpusFileHolder(const char* path);
		bool		 decode();
		void		 getCurrentPointer(float*& data, unsigned int* frameLen);
		void		 moveReadPointer(unsigned int frameLen);
		unsigned int getChannels();
		void		 setJumpPoint(unsigned long point, unsigned long to);
		void		 reset();

		virtual ~OpusFileHolder();

	private:
		static constexpr unsigned int BUFF_FRAME_LEN = 960;

		void*			mFile;
		unsigned int	mChannels;
		float*			mBuffer;
		unsigned int	mFrameLen;
		unsigned int    mReadPointer;
		bool            mNoFinish;
		bool			mJump;
		unsigned long   mJumpTo;
		unsigned long   mJumpPoint;
		unsigned long   mFrameCounter;
	};

	/////////////////////////////////////////////
	class VideoFileHolder {
	public:
		VideoFileHolder(const char* path);
		bool		 decode();

		virtual ~VideoFileHolder();

	private:
		void*		mInstance;
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
}