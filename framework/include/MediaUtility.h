#pragma once

#include "RoseAura.h"

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
		void*	 mImpl;
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

		virtual  ~OpusFileHolder();

	private:
		void*	 mImpl;
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

		virtual ~VideoFileHolder();
	private:
		void* mImpl;
	};


	/////////////////////////////////////////////
	class PreRenderThread {
	public:
		bool	start();
		void	wakeUp();
		void	finish();
		void	finishSelf();

		virtual void doWork() = 0;

		PreRenderThread();
		virtual ~PreRenderThread();
	
	private:
		void*	mImpl;
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

	/////////////////////////////////////////////
	class MovieRenderer {
	public:
		class IMovieRendererCallback {
		public:
			virtual void onVideoFinish() = 0;
		};

		MovieRenderer(RoseAura& ra, const char* movieFile, uint32_t x, uint32_t y,IMovieRendererCallback* cb);

		bool playMovie();
		bool stopPlaying();

		virtual ~MovieRenderer();
	private:
		void* mImpl;
	};
}