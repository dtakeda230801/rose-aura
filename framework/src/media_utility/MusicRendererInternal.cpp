#include "MusicRendererInternal.h"
#include "RoseAura.h"
#include "Utility.h"

bool MusicRendererInternal::playMusic()
{
	ISoundCoordinator& sc = mRa.getSoundCoordinator();

	startDecodeThread();
	sc.registerRenderer(this);

	return true;
}

bool MusicRendererInternal::stopPlaying()
{
	mPlay = false;
	termDecodeThread();

	return true;
}

void MusicRendererInternal::setJumpPoint(uint64_t point, uint64_t to)
{
	mOpusFileHolder->setJumpPoint(point, to);
}

MusicRendererInternal::MusicRendererInternal(RoseAura& ra, const char* musicFile) :
	  mRa(ra)
    , mOpusFileHolder(std::make_unique<OpusFileHolderInternal>(musicFile))
	, mPlay(false)
	, mNoFinish(true)
{
}

void MusicRendererInternal::doWork()
{
	mNoFinish = mOpusFileHolder->decode();
}

MusicRendererInternal::~MusicRendererInternal()
{
	ISoundCoordinator& sc = mRa.getSoundCoordinator();
	sc.unregisterRenderer(this);
	termDecodeThread();
}

RARetCode MusicRendererInternal::requestData(unsigned int requestFrameLen, unsigned int* returnFrameLen, ISoundCoordinator::IDataWriter& writer)
{
	RARetCode ret = RARetCode::RET_OK;

	*returnFrameLen = 0;

	while (*returnFrameLen < requestFrameLen) {

		float* decoded;
		unsigned int	decodedFrameLen;
		unsigned int    writeFrameLen = 0;

		mOpusFileHolder->getCurrentPointer(decoded, &decodedFrameLen);

		if (decodedFrameLen > 0) {
			if (decodedFrameLen >= requestFrameLen - *returnFrameLen) {
				writeFrameLen = requestFrameLen - *returnFrameLen;
			}
			else {
				writeFrameLen = decodedFrameLen;
			}

			writer.write(decoded, writeFrameLen);

			mOpusFileHolder->moveReadPointer(writeFrameLen);

			*returnFrameLen += writeFrameLen;

		}
		else if (mNoFinish) {
			wakeUp();
		}

		if (!mPlay || (!mNoFinish && decodedFrameLen == 0)) {
			ret = RARetCode::RET_END_OF_CONTENT;
			break;
		}
	}
	return ret;
}

void MusicRendererInternal::onAudioStreamFinish()
{
	Utility::printLog("MusicRenderer onFinish");

}

void MusicRendererInternal::startDecodeThread()
{
	mPlay = true;
	mNoFinish = mOpusFileHolder->decode();
	start();
}

void MusicRendererInternal::termDecodeThread()
{
	mPlay     = false;
	mNoFinish = false;
	mOpusFileHolder->reset();
	finish();
}
