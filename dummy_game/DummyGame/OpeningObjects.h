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
		int32_t locY = 0;
		int32_t locU = 0;
		int32_t locV = 0;
		Texture2D texY;
		Texture2D texU;
		Texture2D texV;
		int32_t w = 0;
		int32_t h = 0;

		bool    initialized = false;
		bool    frameReady = false;

		void doPreprocess()
		{
			if (mVideoPoolAvailable < VIDEO_BUFFER_FRAME_LEN) {
				VideoFileHolder::VideoFrame& frame = mVideoPool[mVideoPoolReadPointer];
				++mVideoPoolReadPointer;
				if (mVideoPoolReadPointer == VIDEO_BUFFER_FRAME_LEN) {
					mVideoPoolReadPointer = 0;
				}
				++mVideoPoolAvailable;
				if (!initialized) {
					initialized = true;

					Shader* shader = SHADER_POINTER(mGraphicsManager.getShader(CONVERT_PICTURE_SHADER));

					w = frame.mWidth;
					h = frame.mHeight;

					locY = GetShaderLocation(*shader, "texY");
					locU = GetShaderLocation(*shader, "texU");
					locV = GetShaderLocation(*shader, "texV");

					Image imgY = {
						.data = malloc(w * h),
						.width = w,
						.height = h,
						.mipmaps = 1,
						.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE
					};

					texY = LoadTextureFromImage(imgY);

					Image imgU = {
						.data = malloc(w/2 * h),
						.width = w/2,
						.height = h,
						.mipmaps = 1,
						.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE
					};

					texU = LoadTextureFromImage(imgU);

					Image imgV = {
						.data = malloc(w / 2 * h),
						.width = w / 2,
						.height = h,
						.mipmaps = 1,
						.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE
					};

					texV = LoadTextureFromImage(imgV);
				}

				Utility::printLog("frame: %u x %u", frame.mWidth, frame.mHeight);
				Utility::printLog("texY : %d x %d", texY.width, texY.height);

				UpdateTexture(texY, frame.mY);
				UpdateTexture(texU, frame.mU);
				UpdateTexture(texV, frame.mV);

				mVideoFileHolder->releaseVideoFrame(frame);

				frameReady = true;
			}
		}

		void render()
		{
			if (frameReady) {
				frameReady = false;
				Shader* shader = SHADER_POINTER(mGraphicsManager.getShader(CONVERT_PICTURE_SHADER));

				BeginShaderMode(*shader);

				SetShaderValueTexture(*shader, locY, texY);
				SetShaderValueTexture(*shader, locU, texU);
				SetShaderValueTexture(*shader, locV, texV);

				DrawRectangle(0, 0, w, h, WHITE);

				EndShaderMode();
			}
		};

		RARetCode requestData(unsigned int requestFrameLen, unsigned int* returnFrameLen, ISoundCoordinator::IDataWriter& writer)
		{
			Utility::printLog("requestData in");
			RARetCode ret;

			*(returnFrameLen) = 0;

			while (*(returnFrameLen) < requestFrameLen) {
				uint32_t writeSize;
				float* writePointer;

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
				} else {
					wakeUp();
				}


				if (mAudioWritePointer == mAudioReadPointer) {
					mAudioReadPointer  = 0;
					mAudioWritePointer = 0;
					wakeUp();
				}
			}
			Utility::printLog("requestData out: (%d) (%d)", *(returnFrameLen), requestFrameLen);
			return RARetCode::RET_OK;
		}

		void doWork() {
			Utility::printLog("doWork in");

			if (mDecoderResult == VideoFileHolder::DecoderReturnCode::CONTINUE) {
				mDecoderResult = mVideoFileHolder->decode();
			}

			if (mDecoderResult == VideoFileHolder::DecoderReturnCode::VIDEO) {
				Utility::printLog("VIDEO");
				bool vFrameRet = true;
				while (vFrameRet) {
					if (mVideoPoolAvailable > 0) {
						VideoFileHolder::VideoFrame& frame = mVideoPool[mVideoPoolWritePointer];
						vFrameRet = mVideoFileHolder->getVideoFrame(frame);
						if (vFrameRet) {
							++mVideoPoolWritePointer;
							if (mVideoPoolWritePointer == 5) {
								mVideoPoolWritePointer = 0;
							}
							--mVideoPoolAvailable;
						}
					} else {
						mDecoderResult = VideoFileHolder::DecoderReturnCode::CONTINUE;
						break;
					}
				}
				if (!vFrameRet) {
					mDecoderResult = VideoFileHolder::DecoderReturnCode::CONTINUE;
				}
			}

			if (mDecoderResult == VideoFileHolder::DecoderReturnCode::AUDIO) {
				Utility::printLog("AUDIO");
				bool		aFrameRet = true;
				float*		buffer;
				uint32_t	returnFrameLen;
				uint32_t    requestFrameLen;

				while (aFrameRet) {
					buffer          = mAudioBuffer + (mAudioWritePointer * mVideoFileHolder->getChannels());
					requestFrameLen = AUDIO_BUFFER_FRAME_LEN - mAudioWritePointer;
					aFrameRet       = mVideoFileHolder->getAudioFrame(&buffer, &returnFrameLen, requestFrameLen);

					if (aFrameRet) {
						mAudioWritePointer += returnFrameLen;
					} else {
						mDecoderResult = VideoFileHolder::DecoderReturnCode::CONTINUE;
						break;
					}

					if (mAudioWritePointer == AUDIO_BUFFER_FRAME_LEN) {
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

		{
		}
		virtual ~Movie() = default;

	private:
		static constexpr uint32_t	AUDIO_BUFFER_FRAME_LEN = 960;
		static constexpr uint32_t	VIDEO_BUFFER_FRAME_LEN = 5;

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

