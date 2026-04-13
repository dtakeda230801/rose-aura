#pragma once

#include <queue>

#include "raylib.h"

#include "RoseAura.h"
#include "RoseAuraReturnCode.h"
#include "MediaUtility.h"
#include "Utility.h"

using namespace RoseAuraMediaUtility;
using namespace RoseAuraReturnCode;

namespace OpeningObjects {

	static const IObjectRepository::TAG_ID TAG_OPENING_OBJECT = 0x3;

	static const std::string	CONVERT_PICTURE_SHADER = "ConvertPicture.fs";

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	class Movie : public IGraphicsManager::IObjectRenderer
		        , public ISoundCoordinator::ISoundRenderer
		        , public PreRenderThread
	{
	public:
		void doPreprocess()
		{
			if (mVideoPoolAvailable < VIDEO_BUFFER_FRAME_LEN) {
				VideoFileHolder::VideoFrame& frame = mVideoPool[mVideoPoolReadPointer];
				++mVideoPoolReadPointer;
				if (mVideoPoolReadPointer == VIDEO_BUFFER_FRAME_LEN) {
					mVideoPoolReadPointer = 0;
				}
				++mVideoPoolAvailable;
				if (!mVideoWork.mInitialized) {
					mVideoWork.mInitialized = true;

					Shader* shader = SHADER_POINTER(mGraphicsManager.getShader(CONVERT_PICTURE_SHADER));

					mVideoWork.mWidth  = frame.mWidth;
					mVideoWork.mHeight = frame.mHeight;

					mVideoWork.mLocY = GetShaderLocation(*shader, "texY");
					mVideoWork.mLocU = GetShaderLocation(*shader, "texU");
					mVideoWork.mLocV = GetShaderLocation(*shader, "texV");

					Image imgY = {
						.data = malloc(mVideoWork.mWidth * mVideoWork.mHeight),
						.width = mVideoWork.mWidth,
						.height = mVideoWork.mHeight,
						.mipmaps = 1,
						.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE
					};

					mVideoWork.mTexY = LoadTextureFromImage(imgY);
					free(imgY.data);

					Image imgU = {
						.data = malloc(mVideoWork.mWidth /2 * mVideoWork.mHeight),
						.width = mVideoWork.mWidth /2,
						.height = mVideoWork.mHeight,
						.mipmaps = 1,
						.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE
					};

					mVideoWork.mTexU = LoadTextureFromImage(imgU);
					free(imgU.data);

					Image imgV = {
						.data = malloc(mVideoWork.mWidth / 2 * mVideoWork.mHeight),
						.width = mVideoWork.mWidth / 2,
						.height = mVideoWork.mHeight,
						.mipmaps = 1,
						.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE
					};

					mVideoWork.mTexV = LoadTextureFromImage(imgV);
					free(imgV.data);
				}

				UpdateTexture(mVideoWork.mTexY, frame.mY);
				UpdateTexture(mVideoWork.mTexU, frame.mU);
				UpdateTexture(mVideoWork.mTexV, frame.mV);

				mVideoFileHolder->releaseVideoFrame(frame);
				wakeUp();
			}
		}

		void render()
		{
			if (mVideoWork.mInitialized) {
				Shader* shader = SHADER_POINTER(mGraphicsManager.getShader(CONVERT_PICTURE_SHADER));

				BeginShaderMode(*shader);

				SetShaderValueTexture(*shader, mVideoWork.mLocY, mVideoWork.mTexY);
				SetShaderValueTexture(*shader, mVideoWork.mLocU, mVideoWork.mTexU);
				SetShaderValueTexture(*shader, mVideoWork.mLocV, mVideoWork.mTexV);

				DrawRectangle(0, 0, mVideoWork.mWidth, mVideoWork.mHeight, WHITE);

				EndShaderMode();
			}
		};

		RARetCode requestData(unsigned int requestFrameLen, unsigned int* returnFrameLen, ISoundCoordinator::IDataWriter& writer)
		{
			RARetCode ret;

			*(returnFrameLen) = 0;

			while (*(returnFrameLen) < requestFrameLen) {
				uint32_t writeSize;
				float*   writePointer;

				writeSize    = mAudioWritePointer - mAudioReadPointer;
				writePointer = mAudioBuffer + mAudioReadPointer;

				if (writeSize > requestFrameLen) {
					writeSize = requestFrameLen;
				}

				if (writeSize > 0) {
					ret = writer.write(writePointer, writeSize);
					if (ret == RARetCode::RET_OK) {
						mAudioReadPointer += writeSize;
						*(returnFrameLen) += writeSize;
					}
				}

				if (mAudioWritePointer == mAudioReadPointer) {
					mAudioReadPointer  = 0;
					mAudioWritePointer = 0;
				}
				wakeUp();
			}
			Utility::printLog("requestData out: (%d) (%d)", *(returnFrameLen), requestFrameLen);
			return RARetCode::RET_OK;
		}

		void doWork() {

			if (mDecoderResult == VideoFileHolder::DecoderReturnCode::CONTINUE) {
				mDecoderResult = mVideoFileHolder->decode();
			}

			if (mDecoderResult == VideoFileHolder::DecoderReturnCode::VIDEO) {
				//Utility::printLog("VIDEO");
				bool vFrameRet = true;
				while (vFrameRet) {
					if (mVideoPoolAvailable > 0) {
						VideoFileHolder::VideoFrame& frame = mVideoPool[mVideoPoolWritePointer];
						vFrameRet = mVideoFileHolder->getVideoFrame(frame);
						if (vFrameRet) {
							++mVideoPoolWritePointer;
							if (mVideoPoolWritePointer == VIDEO_BUFFER_FRAME_LEN) {
								mVideoPoolWritePointer = 0;
							}
							--mVideoPoolAvailable;
						}
					} else {
						break;
					}
				}
				if (!vFrameRet) {
					mDecoderResult = VideoFileHolder::DecoderReturnCode::CONTINUE;
				}
			}

			if (mDecoderResult == VideoFileHolder::DecoderReturnCode::AUDIO) {
				//Utility::printLog("AUDIO");
				bool		aFrameRet = true;
				float*      buffer;
				uint32_t	returnFrameLen;
				uint32_t    requestFrameLen;

				while (aFrameRet) {
					if (mAudioWritePointer == AUDIO_BUFFER_FRAME_LEN) {
						break;
					}

					buffer          = mAudioBuffer + (mAudioWritePointer * mVideoFileHolder->getChannels());
					requestFrameLen = AUDIO_BUFFER_FRAME_LEN - mAudioWritePointer;
					aFrameRet       = mVideoFileHolder->getAudioFrame(&buffer, &returnFrameLen, requestFrameLen);

					if (aFrameRet) {
						mAudioBufferMutex.lock();
						mAudioWritePointer += returnFrameLen;
						mAudioBufferMutex.unlock();
					} else {
						mDecoderResult = VideoFileHolder::DecoderReturnCode::CONTINUE;
						break;
					}
				}
			}

			if (mDecoderResult == VideoFileHolder::DecoderReturnCode::FINISH) {
				finishSelf();
			}
		}

		void init()
		{
			mVideoWork.mInitialized = false;

			mAudioBuffer = new float[AUDIO_BUFFER_FRAME_LEN * mVideoFileHolder->getChannels()];
			mAudioDelay = mSoundCoordinator.getDelayTime();
			mGraphicsManager.setRenderer(this);
			mSoundCoordinator.registerRenderer(this);
			startDecodeThread();
		}

		void fin()
		{
			termDecodeThread();
			mGraphicsManager.removeRenderer(this);
			delete[] mAudioBuffer;
			mSoundCoordinator.unregisterRenderer(this);
		}

		void startDecodeThread()
		{
			start();
		}

		void termDecodeThread()
		{
			finish();
		}

		Movie(RoseAura& ra) :
			  mGraphicsManager(ra.getGraphicsManager())
			, mSoundCoordinator(ra.getSoundCoordinator())
			, mDecoderResult(VideoFileHolder::DecoderReturnCode::CONTINUE)
			, mVideoFileHolder(std::make_unique<VideoFileHolder>("test.webm"))
			, mAudioBuffer(nullptr)
			, mAudioWritePointer(0)
			, mAudioReadPointer(0)
			, mAudioDelay(0)
			, mVideoPoolWritePointer(0)
		    , mVideoPoolReadPointer(0)
		    , mVideoPoolAvailable(VIDEO_BUFFER_FRAME_LEN)
			, mVideoWork{}
		{
		}
		virtual ~Movie() = default;

	private:
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

		static constexpr uint32_t	AUDIO_BUFFER_FRAME_LEN = 960*2;
		static constexpr uint32_t	VIDEO_BUFFER_FRAME_LEN = 3;

		IGraphicsManager&  mGraphicsManager;
		ISoundCoordinator& mSoundCoordinator;

		uint32_t		   mAudioDelay;

		std::unique_ptr<VideoFileHolder>
						   mVideoFileHolder;

		float*			   mAudioBuffer;
		uint32_t           mAudioWritePointer;
		uint32_t		   mAudioReadPointer;

		VideoFileHolder::VideoFrame 
					      mVideoPool[VIDEO_BUFFER_FRAME_LEN];
		uint32_t		  mVideoPoolWritePointer;
		uint32_t		  mVideoPoolReadPointer;
		uint32_t          mVideoPoolAvailable;

		VideoFileHolder::DecoderReturnCode
						  mDecoderResult;

		VideoWork		  mVideoWork;

		std::mutex        mAudioBufferMutex;
	};

	//////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////
	std::vector<IObjectRepository::OBJECT_ID> registerObjects(RoseAura& ra)
	{
		std::vector<IObjectRepository::OBJECT_ID> ids;
		std::vector<IObjectRepository::TAG_ID>	  tags = { TAG_OPENING_OBJECT };

		IObjectRepository& objectRepository = ra.getObjectRepository();

		//////////////////////////////////
		ids.push_back(
			objectRepository.registerObject(
				objectRepository.makeObjectBinder<Movie, RoseAura&>(
					&Movie::init, &Movie::fin, ra)
				, tags
			));

		return ids;
	}
}

