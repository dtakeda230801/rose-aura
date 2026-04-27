#pragma once

#include <mutex>
#include <raylib.h>

#include "RoseAura.h"
#include "MediaUtility.h"


namespace RoseAuraMediaUtility {

	class MovieRendererInternal :
		public IGraphicsManager::IGraphicsRenderer
		, public ISoundCoordinator::ISoundRenderer
		, public PreRenderThread
	{
	public:
		/////////////////////////////
		MovieRendererInternal(RoseAura& ra, const char* movieFile, uint32_t x, uint32_t y, MovieRenderer::IMovieRendererCallback* cb);

		bool playMovie();
		bool stopPlaying();

		~MovieRendererInternal();


		/////////////////////////////
		void preprocess();
		void render();

		RARetCode requestData(uint32_t requestFrameLen, uint32_t* returnFrameLen, ISoundCoordinator::IDataWriter& writer);
		void      onAudioStreamFinish();

		void doWork();

	private:
		void waitPreDecode();
		void cleanUpVideoPool();

		struct VideoWork {
			int32_t		mLocY;
			int32_t		mLocU;
			int32_t		mLocV;
			Texture2D	mTexY;
			Texture2D	mTexU;
			Texture2D	mTexV;
			int32_t		mWidth;
			int32_t		mHeight;
			bool        mInitialized;
		};

		static constexpr uint32_t	AUDIO_BUFFER_BLOCK_NUM = 4;
		static constexpr uint32_t	AUDIO_BUFFER_FRAME_LEN = 960;
		static constexpr uint32_t	AUDIO_BUFFER_CHANNELS = 2;
		static constexpr uint32_t	VIDEO_BUFFER_FRAME_LEN = 5;

		static constexpr const char* CONVERT_PICTURE_F_SHADER = "ConvertPictureF.fs";
		static constexpr const char* CONVERT_PICTURE_V_SHADER = "ConvertPictureV.fs";


		IGraphicsManager& mGraphicsManager;
		ISoundCoordinator& mSoundCoordinator;

		MovieRenderer::IMovieRendererCallback*
			mCallback;

		std::unique_ptr<MultiBlockBuffer>
			mMultiBlockBuffer;

		std::unique_ptr<VideoFileHolder>
			mVideoFileHolder;

		VideoFileHolder::VideoFrame
			mVideoPool[VIDEO_BUFFER_FRAME_LEN];
		uint32_t				mVideoPoolWritePointer;
		uint32_t				mVideoPoolReadPointer;
		uint32_t				mVideoPoolAvailable;

		VideoFileHolder::DecoderReturnCode
			mDecoderResult;

		VideoWork				mVideoWork;

		uint32_t				mDispX;
		uint32_t				mDispY;

		IGraphicsManager::SHADER_ID
			mShaderId;

		uint64_t				mBaseTime;
		bool					mFinish;
		std::mutex				mMutex;

	};
};
