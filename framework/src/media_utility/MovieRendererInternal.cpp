#include "MovieRendererInternal.h"

#include "RoseAura.h"
#include "RoseAuraReturnCode.h"

#include "res.h"

#include "Utility.h"

MovieRendererInternal::MovieRendererInternal(RoseAura& ra, const char* movieFile, uint32_t x, uint32_t y, MovieRenderer::IMovieRendererCallback* cb) :
	  mGraphicsManager(ra.getGraphicsManager())
	, mSoundCoordinator(ra.getSoundCoordinator())
	, mDecoderResult(VideoFileHolder::DecoderReturnCode::CONTINUE)
	, mVideoFileHolder(std::make_unique<VideoFileHolder>(movieFile))
	, mMultiBlockBuffer(std::make_unique<MultiBlockBuffer>(AUDIO_BUFFER_BLOCK_NUM, AUDIO_BUFFER_FRAME_LEN, AUDIO_BUFFER_CHANNELS))
	, mVideoPool{}
	, mVideoPoolWritePointer(0)
	, mVideoPoolReadPointer(0)
	, mVideoPoolAvailable(VIDEO_BUFFER_FRAME_LEN)
	, mVideoWork{}
	, mDispX(x)
	, mDispY(y)
	, mShaderId(0)
	, mBaseTime(0)
	, mFinish(false)
	, mCallback(cb)
{

	mGraphicsManager.setShader(RESOURCES->Res_ConvertPictureV
							 , RESOURCES->Res_ConvertPictureF
							 , mShaderId);
}

bool MovieRendererInternal::playMovie()
{
	mVideoWork.mInitialized = false;

	start();
	waitPreDecode();

	mGraphicsManager.setRenderer(this);
	mSoundCoordinator.registerRenderer(this);

    return true;
}

bool MovieRendererInternal::stopPlaying()
{
	mFinish = true;
	finish();
	cleanUpVideoPool();
	mGraphicsManager.removeRenderer(this);
	mSoundCoordinator.unregisterRenderer(this);

    return true;
}

MovieRendererInternal::~MovieRendererInternal()
{
}

void MovieRendererInternal::preprocess()
{
	std::lock_guard<std::mutex> lock(mMutex);

	if (mVideoPoolAvailable < VIDEO_BUFFER_FRAME_LEN) {
		VideoFileHolder::VideoFrame& frame = mVideoPool[mVideoPoolReadPointer];

		uint64_t timming = frame.mTimestamp + mBaseTime;
		if (Utility::getCurrentTime() < timming) {
			return;
		}

		++mVideoPoolReadPointer;
		if (mVideoPoolReadPointer == VIDEO_BUFFER_FRAME_LEN) {
			mVideoPoolReadPointer = 0;
		}
		++mVideoPoolAvailable;
		if (!mVideoWork.mInitialized) {

			Shader* shader = SHADER_POINTER(mGraphicsManager.getShader(mShaderId));
			if (!shader) {
				return;
			}

			mVideoWork.mWidth = frame.mWidth;
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
				.data = malloc(mVideoWork.mWidth / 2 * mVideoWork.mHeight),
				.width = mVideoWork.mWidth / 2,
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

			mVideoWork.mInitialized = true;
		}

		UpdateTexture(mVideoWork.mTexY, frame.mY);
		UpdateTexture(mVideoWork.mTexU, frame.mU);
		UpdateTexture(mVideoWork.mTexV, frame.mV);

		mVideoFileHolder->releaseVideoFrame(frame);
		wakeUp();
	}
	else {
		Utility::printLog("Video Buffer Under run");
		wakeUp();
	}
}

void MovieRendererInternal::render()
{
	if (mVideoWork.mInitialized) {

		Shader* shader = SHADER_POINTER(mGraphicsManager.getShader(mShaderId));

		BeginShaderMode(*shader);

		SetShaderValueTexture(*shader, mVideoWork.mLocY, mVideoWork.mTexY);
		SetShaderValueTexture(*shader, mVideoWork.mLocU, mVideoWork.mTexU);
		SetShaderValueTexture(*shader, mVideoWork.mLocV, mVideoWork.mTexV);

		DrawTexture(mVideoWork.mTexY, mDispX, mDispY, WHITE);

		EndShaderMode();
	}
}

RARetCode MovieRendererInternal::requestData(uint32_t requestFrameLen, uint32_t* returnFrameLen, ISoundCoordinator::IDataWriter& writer)
{
	*(returnFrameLen) = 0;

	while (*(returnFrameLen) < requestFrameLen) {
		float* readBuffer;
		uint32_t	availFrameLen;

		mMultiBlockBuffer->getReadBuffer(readBuffer, availFrameLen);

		if (!readBuffer || availFrameLen == 0) {
			if (mFinish) {
				return RARetCode::RET_END_OF_CONTENT;
			}
			wakeUp();
			continue;
		}

		uint32_t writeFrameLen;
		if (requestFrameLen <= availFrameLen) {
			writeFrameLen = requestFrameLen;
		}
		else {
			writeFrameLen = availFrameLen;
		}

		if (RARetCode::RET_OK != writer.write(readBuffer, writeFrameLen)) {
			Utility::printLog("writer returned error");
			continue;
		}

		mMultiBlockBuffer->updateReadBuffer(writeFrameLen, nullptr);
		*(returnFrameLen) += writeFrameLen;
	}
	wakeUp();

	if (mBaseTime == 0) {
		mBaseTime = mSoundCoordinator.getDelayTime() + Utility::getCurrentTime();
	}

	return RARetCode::RET_OK;
}

void MovieRendererInternal::onAudioStreamFinish()
{
	Utility::printLog("Movie onFinish");
	if (mCallback) {
		mCallback->onVideoFinish();
	}
}

void MovieRendererInternal::doWork()
{

	if (mDecoderResult == VideoFileHolder::DecoderReturnCode::CONTINUE) {
		mDecoderResult = mVideoFileHolder->decode();
	}

	if (mDecoderResult == VideoFileHolder::DecoderReturnCode::VIDEO) {
		//Utility::printLog("VIDEO");
		bool vFrameRet = true;
		while (vFrameRet) {
			if (mVideoPoolAvailable > 0) {
				std::lock_guard<std::mutex> lock(mMutex);

				VideoFileHolder::VideoFrame& frame = mVideoPool[mVideoPoolWritePointer];
				vFrameRet = mVideoFileHolder->getVideoFrame(frame);
				if (vFrameRet) {
					++mVideoPoolWritePointer;
					if (mVideoPoolWritePointer == VIDEO_BUFFER_FRAME_LEN) {
						mVideoPoolWritePointer = 0;
					}
					--mVideoPoolAvailable;
				}
			}
			else {
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
		float* buffer;
		uint32_t	returnFrameLen;
		uint32_t    requestFrameLen;

		while (aFrameRet) {

			mMultiBlockBuffer->getWriteBuffer(buffer, requestFrameLen);

			if (!buffer || requestFrameLen == 0) {
				break;
			}

			aFrameRet = mVideoFileHolder->getAudioFrame(&buffer, &returnFrameLen, requestFrameLen);

			if (aFrameRet) {
				mMultiBlockBuffer->updateWriteBuffer(returnFrameLen, nullptr);
			}
			else {
				mDecoderResult = VideoFileHolder::DecoderReturnCode::CONTINUE;
				break;
			}
		}
	}

	if (mDecoderResult == VideoFileHolder::DecoderReturnCode::FINISH) {
		mFinish = true;
		finishSelf();
	}
}


void MovieRendererInternal::waitPreDecode()
{
	uint32_t count = 0;

	while (count < AUDIO_BUFFER_BLOCK_NUM) {
		wakeUp();
		count = mMultiBlockBuffer->getAvailBlockNum();
	}
}

void MovieRendererInternal::cleanUpVideoPool()
{
	std::lock_guard<std::mutex> lock(mMutex);

	while (mVideoPoolAvailable < VIDEO_BUFFER_FRAME_LEN) {
		VideoFileHolder::VideoFrame& frame = mVideoPool[mVideoPoolReadPointer];

		mVideoFileHolder->releaseVideoFrame(frame);

		++mVideoPoolReadPointer;
		if (mVideoPoolReadPointer == VIDEO_BUFFER_FRAME_LEN) {
			mVideoPoolReadPointer = 0;
		}
		++mVideoPoolAvailable;
	}
}