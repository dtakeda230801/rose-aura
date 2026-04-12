#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>
#include <algorithm>

#include <opus/opusfile.h>
#include "mkvparser/mkvparser.h"
#include "mkvparser/mkvreader.h"

extern "C" {
#include <dav1d/dav1d.h>
#include <opus/opus.h>
}

#include "Utility.h"
#include "MediaUtility.h"

#include "MediaUtilityImpl.h"

using namespace RoseAuraMediaUtility;

////////////////////////////////////////////////////
////////////////////////////////////////////////////
WaveFileHolder::WaveFileHolder(const char* path) :
      mChannels(0)
    , mSamplingRate(0)
    , mFrameLen(0)
    , mCurrentFrame(0)
    , mData(nullptr)

{
    char  riff[4];
    char  size[4];
    char  wave[4];
    char  chunk[4];
    int   chunkSize;
    short audioFormat;
    short channels;
    int   sampleRate;
    short bitsPerSample;
    int   dataSize;
    int   sampleCount;

    std::ifstream waveFile(path, std::ios::binary);
    if (!waveFile)
    {
        return;
    }

    waveFile.read(riff, 4);
    waveFile.read(size, 4);
    waveFile.read(wave, 4);

    while (true) {
        waveFile.read(chunk, 4);
        waveFile.read((char*)&chunkSize, 4);
        if (memcmp(chunk, "fmt ", 4) == 0) {
            break;
        }
        waveFile.ignore(chunkSize);
    }

    waveFile.read((char*)&audioFormat, 2);
    waveFile.read((char*)&channels, 2);
    waveFile.read((char*)&sampleRate, 4);
    waveFile.ignore(6);
    waveFile.read((char*)&bitsPerSample, 2);

    if (chunkSize > 16) {
        waveFile.ignore(chunkSize - 16);
    }

    while (true) {
        waveFile.read(chunk, 4);
        waveFile.read((char*)&chunkSize, 4);
        if (memcmp(chunk, "data", 4) == 0) {
            break;
        }
        waveFile.ignore(chunkSize);
    }

    dataSize = chunkSize;

    sampleCount = dataSize / (bitsPerSample / 8);

    mData = new float[sampleCount];

    if (bitsPerSample == 16) {
        std::vector<short> tmp(sampleCount);
        waveFile.read((char*)tmp.data(), dataSize);

        for (int i = 0; i < sampleCount; ++i)
            mData[i] = tmp[i] / 32768.0f;
    }
    else if (bitsPerSample == 32) {
        waveFile.read((char*)mData, dataSize);
    }
    else {
        return;
    }

    mChannels     = channels;
    mSamplingRate = sampleRate;
    mFrameLen     = static_cast<unsigned int>(sampleCount) / channels;
    mCurrentFrame = 0;
}

float* WaveFileHolder::getCurrentFramePointer()
{
    return &mData[mCurrentFrame * mChannels];
}

void WaveFileHolder::moveCurrentFramePointer(unsigned int frameLen)
{
    mCurrentFrame += frameLen;

    if (frameLen > mFrameLen - 1) {
        mCurrentFrame = mFrameLen - 1;
    }
}

unsigned int WaveFileHolder::getFrameLen()
{
    return mFrameLen;
}

unsigned int WaveFileHolder::getRemainFrameLen()
{
    return mFrameLen - mCurrentFrame;
}


unsigned int WaveFileHolder::getSamplingRate()
{
    return mSamplingRate;
}

unsigned int WaveFileHolder::getChannelNum()
{
    return mChannels;
}

void WaveFileHolder::reset()
{
    mCurrentFrame = 0;
}


WaveFileHolder::~WaveFileHolder()
{
    if (mData) {
        delete mData;
    }
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
OpusFileHolder::OpusFileHolder(const char* path) :
      mFile(nullptr)
    , mChannels(0)
    , mBuffer(nullptr)
    , mFrameLen(0)
    , mReadPointer(0)
    , mNoFinish(true)
    , mJump(false)
    , mJumpTo(0)
    , mJumpPoint(0)
    , mFrameCounter(0)
{
    int ret;
    OggOpusFile* file;

    file = op_open_file(path, &ret);
    if (!file) {
        Utility::printLog("op_open_file returns error(%d)", ret);
        return;
    }
    
    ret = op_channel_count(file, -1);
    if (ret == OP_EINVAL) {
        Utility::printLog("op_channel_count returns error(%d)", ret);
        return;
    }

    mFile     = static_cast<void*>(file);
    mChannels = static_cast<unsigned int>(ret);
    mBuffer   = new float[BUFF_FRAME_LEN * mChannels];
}

bool OpusFileHolder::decode()
{
    OggOpusFile* file = static_cast<OggOpusFile*>(mFile);

    if (mFrameLen == 0) {
        int ret;

        ret = op_read_float(file, mBuffer, BUFF_FRAME_LEN * mChannels, nullptr);
        if (ret > 0) {
            mFrameLen = ret;

            if (mJump) {
                mFrameCounter += ret;
                if (mJumpPoint <= mFrameCounter) {
                    mFrameLen -= (mFrameCounter - mJumpPoint);

                    unsigned int seekPoint;
                    unsigned int preDecodeLen;
                    if (mJumpTo < BUFF_FRAME_LEN) {
                        seekPoint    = 0;
                        preDecodeLen = mJumpTo;
                    } else {
                        seekPoint    = mJumpTo - BUFF_FRAME_LEN;
                        preDecodeLen = BUFF_FRAME_LEN;
                    }
                    op_pcm_seek(file, seekPoint);

                    unsigned int preDecodeOut = 0;
                    float* preDecodeBuffer = new float[preDecodeLen*mChannels];
                    while (preDecodeOut < preDecodeLen) {
                        preDecodeOut += op_read_float(file, preDecodeBuffer, preDecodeLen, nullptr);
                    }
                    delete[] preDecodeBuffer;
                    mFrameCounter = mJumpTo;
                }
            }
        }

        if (ret == 0) {
            mNoFinish = false;
        }
    }

    return mNoFinish;
}

void OpusFileHolder::getCurrentPointer(float*& data, unsigned int* frameLen)
{

    data      = mBuffer + mReadPointer * mChannels;
    *frameLen = mFrameLen;
}

void OpusFileHolder::moveReadPointer(unsigned int frameLen)
{
    mReadPointer += frameLen;
    if (mReadPointer >= mFrameLen) {
        mFrameLen    = 0;
        mReadPointer = 0;
    }
}

unsigned int OpusFileHolder::getChannels()
{
    return mChannels;
}

void OpusFileHolder::setJumpPoint(unsigned long point, unsigned long to)
{
    mJump      = true;
    mJumpPoint = point;
    mJumpTo = to;
}


void OpusFileHolder::reset()
{
    OggOpusFile* file = static_cast<OggOpusFile*>(mFile);

    op_pcm_seek(file, 0);
    mFrameLen     = 0;
    mReadPointer  = 0;
    mNoFinish     = true;
    mJump         = false;
    mJumpTo    = 0;
    mJumpPoint      = 0;
    mFrameCounter = 0;
}

OpusFileHolder::~OpusFileHolder()
{
    if (mFile) {
        OggOpusFile* file = static_cast<OggOpusFile*>(mFile);
        op_free(file);
        mFile = nullptr;
    }

    if (mBuffer) {
        delete[] mBuffer;
    }
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
VideoFileHolder::VideoFileHolder(const char* path) :
    mImpl(std::make_unique<VideoFileHolder::VideoFileHolderImpl>(path))
{
}

VideoFileHolder::DecoderReturnCode VideoFileHolder::decode()
{
    return mImpl->decode();
}

bool VideoFileHolder::getAudioFrame(float** buff, unsigned int* returnFrameLen, unsigned int requestFrameLen)
{
    return mImpl->getAudioFrame(buff, returnFrameLen, requestFrameLen);
}

bool VideoFileHolder::getVideoFrame(VideoFrame& videoFrame)
{
    return mImpl->getVideoFrame(videoFrame);
}

void VideoFileHolder::releaseVideoFrame(VideoFrame& frame)
{
    mImpl->releaseVideoFrame(frame);
}

uint32_t VideoFileHolder::getSamplingRate()
{
    return mImpl->getSamplingRate();

}
uint32_t VideoFileHolder::getChannels()
{
    return mImpl->getChannels();
}


////////////////////////////////////////////////////

VideoFileHolder::VideoFileHolderImpl::VideoFileHolderImpl(const char* path) :
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

    unsigned long long   priv_size;
    const unsigned char* priv = mVideoTrack->GetCodecPrivate(priv_size);

    std::vector<unsigned char> out;
    out = extractSequenceHeaderOBU(priv, static_cast<unsigned int>(priv_size));
    sendToDav1d(out.data(), static_cast<unsigned int>(priv_size));

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

    mSamplingRate = static_cast<unsigned int>(audioTrack->GetSamplingRate());
    mChannels     = static_cast<unsigned int>(audioTrack->GetChannels());

    int opus_err = 0;
    mOpusDecoder = opus_decoder_create(mSamplingRate, mChannels, &opus_err);
    if (opus_err != OPUS_OK) {
        Utility::printLog("opus init failed\n");
        return;
    }

    mAudioBuffer = new float[AUDIO_BUFFER_FRAME_LEN * mChannels];

}

VideoFileHolder::DecoderReturnCode VideoFileHolder::VideoFileHolderImpl::decode()
{
    DecoderReturnCode ret = DecoderReturnCode::CONTINUE;

    if (mPictureReady || mAudioReady) {
        return DecoderReturnCode::CONTINUE;
    }

    if (!mCluster || mCluster->EOS()) {
        return DecoderReturnCode::FINISH;
    }

    if (!mBlockEntry) {
        if (mCluster->GetFirst(mBlockEntry)) {
            return DecoderReturnCode::CONTINUE;
        }
        /*
        if (!mBlockEntry || mBlockEntry->EOS()) {
            mCluster = mSegment->GetNext(mCluster);
            return DecoderReturnCode::CONTINUE;
        }
        */
    }

    const mkvparser::Block* block = mBlockEntry->GetBlock();
    const mkvparser::Track* track = mSegment->GetTracks()->GetTrackByNumber(static_cast<long>(block->GetTrackNumber()));

    unsigned int frameLen = block->GetFrameCount();

    for (mFrameCount; mFrameCount < frameLen; ++mFrameCount) {

        const mkvparser::Block::Frame& frame = block->GetFrame(mFrameCount);

        std::vector<uint8_t> buf(frame.len);
        if (mReader.Read(frame.pos, frame.len, buf.data())) continue;

        //////////////////////////////////////
        if (track == mVideoTrack) {
            sendToDav1d(buf.data(), static_cast<unsigned int>(buf.size()));

            if (getPicture()) {
                //Utility::printLog("video frame: %d x %d", mPicture.p.w, mPicture.p.h);
                mPictureReady = true;
                ret = DecoderReturnCode::VIDEO;
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
                //Utility::printLog("audio frame: %d frame", mAudioFrameLen);
                mAudioReady = true;
                mFrameCount++;
                ret = DecoderReturnCode::AUDIO;
                break;
            }
        }
    }

    if (mFrameCount == frameLen) {
        if (mCluster->GetNext(mBlockEntry, mBlockEntry) || !mBlockEntry || mBlockEntry->EOS()) {
            mBlockEntry = nullptr;
            mCluster    = mSegment->GetNext(mCluster);
        }
        mFrameCount = 0;
    }

    return ret;
}

bool VideoFileHolder::VideoFileHolderImpl::getAudioFrame(float** buff, unsigned int* returnFrameLen, unsigned int requestFrameLen)
{
    bool ret = false;

    if (mAudioReady) {
        float*          srcBuff  = mAudioBuffer + (mAudioReadPointer * mChannels);
        float*          outBuff  = *buff;
        unsigned int    outCount = 0;

        *returnFrameLen = 0;

        if (mAudioFrameLen - mAudioReadPointer < requestFrameLen) {
            outCount = mAudioFrameLen - mAudioReadPointer;
        } else {
            outCount = requestFrameLen;
        }
        for (unsigned int i = 0; i < outCount; ++i) {
            for (unsigned int ch = 0; ch < mChannels; ++ch) {
                *outBuff++ = *srcBuff++;
            }
            ++(*returnFrameLen);
        }

        mAudioReadPointer += *returnFrameLen;

        if (mAudioReadPointer >= mAudioFrameLen) {
            mAudioReadPointer = 0;
            mAudioFrameLen    = 0;
            mAudioReady       = 0;
        }
        ret = true;
    }

    return ret;
}

bool VideoFileHolder::VideoFileHolderImpl::getVideoFrame(VideoFrame& videoFrame)
{
    if (mPictureReady) {
        videoFrame = convertPictureFormat(mPicture);
        dav1d_picture_unref(&mPicture);
        mPictureReady = false;
        return true;
    } else {
        if (getPicture()) {
            //Utility::printLog("video frame: %d x %d", mPicture.p.w, mPicture.p.h);
            videoFrame = convertPictureFormat(mPicture);
            dav1d_picture_unref(&mPicture);
            return true;
        }
    }

    return false;
}

void VideoFileHolder::VideoFileHolderImpl::releaseVideoFrame(VideoFrame& frame) {
    delete[] frame.mY;
    delete[] frame.mU;
    delete[] frame.mV;
    frame.mY = nullptr;
    frame.mU = nullptr;
    frame.mY = nullptr;
}

uint32_t VideoFileHolder::VideoFileHolderImpl::getSamplingRate()
{
    return mSamplingRate;

}
uint32_t VideoFileHolder::VideoFileHolderImpl::getChannels()
{
    return mChannels;
}


VideoFileHolder::VideoFileHolderImpl::~VideoFileHolderImpl()
{
    opus_decoder_destroy(mOpusDecoder);
    dav1d_close(&mDav1dContext);
    delete mSegment;
    mReader.Close();

    delete[] mAudioBuffer;
}

std::vector<unsigned char>
VideoFileHolder::VideoFileHolderImpl::extractSequenceHeaderOBU(const unsigned char* priv, unsigned int size)
{
    std::vector<unsigned char> out;

    if (!priv || size < 4)
        return out;

    unsigned int pos = 4;

    while (pos < size)
    {
        out.push_back(priv[pos++]);
    }

    return out;
}

void VideoFileHolder::VideoFileHolderImpl::sendToDav1d(const unsigned char* data, unsigned int size)
{
    Dav1dData d = { 0 };

    unsigned char* heap = new unsigned char[size];
    memcpy(heap, data, size);

    dav1d_data_wrap(
        &d,
        heap,
        size,
        [](const unsigned char* data, void*) {
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
VideoFileHolder::VideoFileHolderImpl::convertPictureFormat(const Dav1dPicture& pic)
{
    VideoFileHolder::VideoFrame frame;

    frame.mWidth  = pic.p.w;
    frame.mHeight = pic.p.h;

    frame.mY = new uint8_t[frame.mWidth * frame.mHeight];

    for (uint32_t y = 0; y < frame.mHeight; y++) {
        memcpy(frame.mY + y * frame.mWidth
             , static_cast<uint8_t*>(pic.data[0]) + y * pic.stride[0]
             , frame.mWidth);
    }

    uint32_t uw = frame.mWidth / 2;
    uint32_t uh = frame.mHeight;

    frame.mU = new uint8_t[uw * uh];
    frame.mV = new uint8_t[uw * uh];

    for (uint32_t y = 0; y < uh; y++)
    {
        memcpy(frame.mU + y * uw, static_cast<uint8_t*>(pic.data[1]) + y * pic.stride[1], uw);
        memcpy(frame.mV + y * uw, static_cast<uint8_t*>(pic.data[2]) + y * pic.stride[1], uw);
    }
/*
    uint32_t ySize = static_cast<uint32_t>( pic.p.h      * pic.stride[0]);
    uint32_t uSize = static_cast<uint32_t>((pic.p.h / 2) * pic.stride[1]);
    uint32_t vSize = static_cast<uint32_t>((pic.p.h / 2) * pic.stride[1]);

    frame.mY = new uint8_t[ySize];
    frame.mU = new uint8_t[uSize];
    frame.mV = new uint8_t[vSize];
    
    uint8_t* srcStart;
    uint8_t* srcEnd;

    srcStart = static_cast<uint8_t*>(pic.data[0]);
    srcEnd   = static_cast<uint8_t*>(pic.data[0]) + ySize;
    std::copy(srcStart, srcEnd, frame.mY);

    srcStart = static_cast<uint8_t*>(pic.data[1]);
    srcEnd   = static_cast<uint8_t*>(pic.data[1]) + uSize;
    std::copy(srcStart, srcEnd, frame.mU);

    srcStart = static_cast<uint8_t*>(pic.data[2]);
    srcEnd   = static_cast<uint8_t*>(pic.data[2]) + vSize;
    std::copy(srcStart, srcEnd, frame.mV);
*/
    frame.mStrideY = static_cast<uint32_t>(pic.stride[0]);
    frame.mStrideU = static_cast<uint32_t>(pic.stride[1]);
    frame.mStrideV = static_cast<uint32_t>(pic.stride[1]);

    frame.mTimestamp = pic.m.timestamp;

    return frame;
}

bool VideoFileHolder::VideoFileHolderImpl::getPicture()
{   
    if (dav1d_get_picture(mDav1dContext, &mPicture) == 0) {
        return true;
    }
    return false;
}


////////////////////////////////////////////////////
////////////////////////////////////////////////////
bool PreRenderThread::start()
{
    if (mStarted.load(std::memory_order_acquire)) {
        return false;
    }
    mThread = std::thread(&PreRenderThread::threadFunc, this);
    mStarted.wait(false);
    return true;
}

void PreRenderThread::wakeUp()
{
    mSem.release();
}

void PreRenderThread::finish()
{
    mStarted.store(false, std::memory_order_release);
    mSem.release();

    if (mThread.joinable()) {
        mThread.join();
    }
}

void PreRenderThread::finishSelf()
{
    mStarted.store(false, std::memory_order_release);
}


PreRenderThread::PreRenderThread() :
      mStarted(false)
    , mSem(0)
{
}

void PreRenderThread::threadFunc()
{
    mStarted.store(true, std::memory_order_release);
    mStarted.notify_one();

    while (mStarted.load(std::memory_order_acquire)) {
        doWork();
        mSem.acquire(); 
    }
}
