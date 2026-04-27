#include <opus/opusfile.h>
#include "mkvparser/mkvparser.h"
#include "mkvparser/mkvreader.h"

extern "C" {
#include <dav1d/dav1d.h>
#include <opus/opus.h>
}

#include "VideoFileHolderInternal.h"
#include "Utility.h"

using namespace RoseAuraMediaUtility;

VideoFileHolderInternal::VideoFileHolderInternal(const char* path) :
      mOpusDecoder(nullptr)
    , mSegment(nullptr)
    , mCluster(nullptr)
    , mVideoTrack(nullptr)
    , mAudioTrack(nullptr)
    , mDav1dContext(nullptr)
    , mBlockEntry(nullptr)
    , mFrameCount(0)
    , mPicture{}
    , mPictureReady(false)
    , mLatestTimeStamp(0)
    , mAudioBuffer(nullptr)
    , mSamplingRate(0)
    , mChannels(0)
    , mAudioReady(false)
    , mAudioFrameLen(0)
    , mAudioReadPointer(0)
{
    long long pos = 0;

    if (mReader.Open(path)) {
        Utility::printLog("open failed");
        return;
    }

    mkvparser::EBMLHeader ebml;
    if (ebml.Parse(&mReader, pos)) {
        Utility::printLog("EBML parse failed");
        return;
    }

    if (mkvparser::Segment::CreateInstance(&mReader, pos, mSegment)) {
        Utility::printLog("segment create failed");
        return;
    }

    if (mSegment->Load()) {
        Utility::printLog("segment load failed");
        return;
    }

    const mkvparser::Tracks* tracks = mSegment->GetTracks();

    for (unsigned i = 0; i < tracks->GetTracksCount(); ++i) {
        const mkvparser::Track* track = tracks->GetTrackByIndex(i);
        if (track->GetType() == mkvparser::Track::kVideo) {
            mVideoTrack = track;
        }
        if (track->GetType() == mkvparser::Track::kAudio) {
            mAudioTrack = track;
        }
    }

    if (!mVideoTrack || !mAudioTrack) {
        Utility::printLog("Can not find suitable track.");
        return;
    }

    Dav1dSettings dav1d_settings;
    dav1d_default_settings(&dav1d_settings);

    if (dav1d_open(&mDav1dContext, &dav1d_settings) < 0) {
        Utility::printLog("dav1d init failed");
        return;
    }

    uint64_t priv_size;
    const uint8_t* priv = mVideoTrack->GetCodecPrivate(priv_size);

    std::vector<uint8_t> out;
    out = extractSequenceHeaderOBU(priv, static_cast<uint32_t>(priv_size));
    sendToDav1d(out.data(), static_cast<uint32_t>(priv_size));

    mCluster = mSegment->GetFirst();
    if (!mCluster) {
        Utility::printLog("Can not get first cluster");
        return;
    }


    if (mCluster->GetFirst(mBlockEntry)) {
        Utility::printLog("Can not get first block entry");
        return;
    }

    ///////////////////////////////////////////////
    const mkvparser::AudioTrack* audioTrack =
        static_cast<const mkvparser::AudioTrack*>(mAudioTrack);

    mSamplingRate = static_cast<uint32_t>(audioTrack->GetSamplingRate());
    mChannels = static_cast<uint32_t>(audioTrack->GetChannels());

    int opus_err = 0;
    mOpusDecoder = opus_decoder_create(mSamplingRate, mChannels, &opus_err);
    if (opus_err != OPUS_OK) {
        Utility::printLog("opus init failed\n");
        return;
    }

    mAudioBuffer = new float[AUDIO_BUFFER_FRAME_LEN * mChannels];

}

VideoFileHolder::DecoderReturnCode VideoFileHolderInternal::decode()
{
    VideoFileHolder::DecoderReturnCode ret = VideoFileHolder::DecoderReturnCode::CONTINUE;

    if (mPictureReady || mAudioReady) {
        return VideoFileHolder::DecoderReturnCode::CONTINUE;
    }

    if (!mCluster || mCluster->EOS()) {
        return VideoFileHolder::DecoderReturnCode::FINISH;
    }

    if (!mBlockEntry) {
        if (mCluster->GetFirst(mBlockEntry)) {
            return VideoFileHolder::DecoderReturnCode::CONTINUE;
        }
    }

    const mkvparser::Block* block = mBlockEntry->GetBlock();
    const mkvparser::Track* track = mSegment->GetTracks()->GetTrackByNumber(static_cast<long>(block->GetTrackNumber()));

    uint32_t frameLen = block->GetFrameCount();

    for (mFrameCount; mFrameCount < frameLen; ++mFrameCount) {

        const mkvparser::Block::Frame& frame = block->GetFrame(mFrameCount);
        mLatestTimeStamp = block->GetTime(mCluster) / 1000;

        std::vector<uint8_t> buf(frame.len);
        if (mReader.Read(frame.pos, frame.len, buf.data())) continue;

        //////////////////////////////////////
        if (track == mVideoTrack) {
            sendToDav1d(buf.data(), static_cast<uint32_t>(buf.size()));

            if (getPicture()) {
                mPictureReady = true;
                ret = VideoFileHolder::DecoderReturnCode::VIDEO;
                mFrameCount++;
                break;
            }
        }

        //////////////////////////////////////
        if (track == mAudioTrack) {
            mAudioFrameLen = opus_decode_float(
                mOpusDecoder
                , buf.data()
                , static_cast<opus_int32>(buf.size())
                , mAudioBuffer
                , AUDIO_BUFFER_FRAME_LEN
                , 0);

            if (mAudioFrameLen > 0) {
                mAudioReady = true;
                mFrameCount++;
                ret = VideoFileHolder::DecoderReturnCode::AUDIO;
                break;
            }
        }
    }

    if (mFrameCount == frameLen) {
        if (mCluster->GetNext(mBlockEntry, mBlockEntry) || !mBlockEntry || mBlockEntry->EOS()) {
            mBlockEntry = nullptr;
            mCluster = mSegment->GetNext(mCluster);
        }
        mFrameCount = 0;
    }

    return ret;
}

bool VideoFileHolderInternal::getAudioFrame(float** buff, uint32_t* returnFrameLen, uint32_t requestFrameLen)
{
    bool ret = false;

    if (mAudioReady) {
        float* srcBuff = mAudioBuffer + (mAudioReadPointer * mChannels);
        float* outBuff = *buff;
        uint32_t    outCount = 0;

        *returnFrameLen = 0;

        if (mAudioFrameLen - mAudioReadPointer < requestFrameLen) {
            outCount = mAudioFrameLen - mAudioReadPointer;
        }
        else {
            outCount = requestFrameLen;
        }
        for (uint32_t i = 0; i < outCount; ++i) {
            for (uint32_t ch = 0; ch < mChannels; ++ch) {
                *outBuff++ = *srcBuff++;
            }
            ++(*returnFrameLen);
        }

        mAudioReadPointer += *returnFrameLen;

        if (mAudioReadPointer >= mAudioFrameLen) {
            mAudioReadPointer = 0;
            mAudioFrameLen = 0;
            mAudioReady = 0;
        }
        ret = true;
    }

    return ret;
}

bool VideoFileHolderInternal::getVideoFrame(VideoFileHolder::VideoFrame& videoFrame)
{
    if (mPictureReady) {
        videoFrame = convertPictureFormat(mPicture);
        dav1d_picture_unref(&mPicture);
        mPictureReady = false;
        return true;
    }
    else {
        if (getPicture()) {
            videoFrame = convertPictureFormat(mPicture);
            dav1d_picture_unref(&mPicture);
            return true;
        }
    }

    return false;
}

void VideoFileHolderInternal::releaseVideoFrame(VideoFileHolder::VideoFrame& frame) {
    delete[] frame.mY;
    delete[] frame.mU;
    delete[] frame.mV;
    frame.mY = nullptr;
    frame.mU = nullptr;
    frame.mY = nullptr;
}

uint32_t VideoFileHolderInternal::getSamplingRate()
{
    return mSamplingRate;

}
uint32_t VideoFileHolderInternal::getChannels()
{
    return mChannels;
}


VideoFileHolderInternal::~VideoFileHolderInternal()
{
    opus_decoder_destroy(mOpusDecoder);
    dav1d_close(&mDav1dContext);
    delete mSegment;
    mReader.Close();

    delete[] mAudioBuffer;
}

std::vector<uint8_t>
VideoFileHolderInternal::extractSequenceHeaderOBU(const uint8_t* priv, uint32_t size)
{
    std::vector<uint8_t> out;

    if (!priv || size < 4)
        return out;

    uint32_t pos = 4;

    while (pos < size)
    {
        out.push_back(priv[pos++]);
    }

    return out;
}

void VideoFileHolderInternal::sendToDav1d(const uint8_t* data, uint32_t size)
{
    Dav1dData d = { 0 };

    uint8_t* heap = new uint8_t[size];
    memcpy(heap, data, size);

    dav1d_data_wrap(
        &d,
        heap,
        size,
        [](const uint8_t* data, void*) {
            delete[] data;
        },
        nullptr
    );

    int res = dav1d_send_data(mDav1dContext, &d);
    if (res < 0)
    {
        Utility::printLog("send error: (%d)", res);
    }
}

VideoFileHolder::VideoFrame
VideoFileHolderInternal::convertPictureFormat(const Dav1dPicture& pic)
{
    VideoFileHolder::VideoFrame frame;

    uint16_t* srcY;
    uint16_t* srcU;
    uint16_t* srcV;
    uint32_t  stride;

    frame.mWidth = pic.p.w;
    frame.mHeight = pic.p.h;
    //////////////////////////
    frame.mY = new uint8_t[frame.mWidth * frame.mHeight];

    srcY = static_cast<uint16_t*>(pic.data[0]);
    stride = static_cast<uint32_t>(pic.stride[0]) / 2;

    for (uint32_t y = 0; y < frame.mHeight; y++) {
        for (uint32_t x = 0; x < frame.mWidth; x++) {
            uint16_t val = srcY[y * stride + x];
            frame.mY[y * frame.mWidth + x] = static_cast<uint8_t>(val >> 2);
        }
    }
    //////////////////////////
    frame.mU = new uint8_t[frame.mWidth / 2 * frame.mHeight];
    frame.mV = new uint8_t[frame.mWidth / 2 * frame.mHeight];

    srcU = static_cast<uint16_t*>(pic.data[1]);
    srcV = static_cast<uint16_t*>(pic.data[2]);
    stride = static_cast<uint32_t>(pic.stride[1]) / 2;

    for (uint32_t y = 0; y < frame.mHeight; y++) {
        for (uint32_t x = 0; x < frame.mWidth / 2; x++) {
            frame.mU[y * frame.mWidth / 2 + x] = (uint8_t)(srcU[y * stride + x] >> 2);
            frame.mV[y * frame.mWidth / 2 + x] = (uint8_t)(srcV[y * stride + x] >> 2);
        }
    }

    frame.mTimestamp = mLatestTimeStamp;

    return frame;
}

bool VideoFileHolderInternal::getPicture()
{
    if (dav1d_get_picture(mDav1dContext, &mPicture) == 0) {
        return true;
    }
    return false;
}
