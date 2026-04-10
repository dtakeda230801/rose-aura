#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>

#include <opus/opusfile.h>
#include "mkvparser/mkvparser.h"
#include "mkvparser/mkvreader.h"

extern "C" {
#include <dav1d/dav1d.h>
#include <opus/opus.h>
}

#include "Utility.h"
#include "MediaUtility.h"

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
struct VideoFileHolderInstance {
    mkvparser::MkvReader        mReader;
    OpusDecoder*                mOpusDecoder;
    mkvparser::Segment*         mSegment;
    const mkvparser::Cluster*   mCluster;
    const mkvparser::Track*     mVideoTrack;
    const mkvparser::Track*     mAudioTrack;
    Dav1dContext*               mDav1dContext;


};


VideoFileHolder::VideoFileHolder(const char* path) :
    mInstance(static_cast<void*>(new VideoFileHolderInstance()))
{
    VideoFileHolderInstance* ins = static_cast<VideoFileHolderInstance*>(mInstance);

    long long pos = 0;

    if (ins->mReader.Open(path)) {
        Utility::printLog("open failed");
        return;
    }

    mkvparser::EBMLHeader ebml;
    if (ebml.Parse(&ins->mReader, pos)) {
        Utility::printLog("EBML parse failed");
        return;
    }

    ins->mSegment = nullptr;
    if (mkvparser::Segment::CreateInstance(&ins->mReader, pos, ins->mSegment)) {
        Utility::printLog("segment create failed");
        return;
    }

    if (ins->mSegment->Load()) {
        Utility::printLog("segment load failed");
        return;
    }

    const mkvparser::Tracks* tracks = ins->mSegment->GetTracks();

    const mkvparser::Track* video_track = nullptr;
    const mkvparser::Track* audio_track = nullptr;

    for (unsigned i = 0; i < tracks->GetTracksCount(); ++i) {
        const mkvparser::Track* t = tracks->GetTrackByIndex(i);
        if (t->GetType() == mkvparser::Track::kVideo) ins->mVideoTrack = t;
        if (t->GetType() == mkvparser::Track::kAudio) ins->mAudioTrack = t;
    }

    Dav1dSettings dav1d_settings;
    dav1d_default_settings(&dav1d_settings);

    ins->mDav1dContext = nullptr;
    if (dav1d_open(&ins->mDav1dContext, &dav1d_settings) < 0) {
        Utility::printLog("dav1d init failed");
        return;
    }

    int opus_err = 0;
    ins->mOpusDecoder = opus_decoder_create(48000, 2, &opus_err);
    if (opus_err != OPUS_OK) {
        Utility::printLog("opus init failed\n");
        return;
    }

    ins->mCluster = ins->mSegment->GetFirst();

}

bool VideoFileHolder::decode()
{
    VideoFileHolderInstance* ins = static_cast<VideoFileHolderInstance*>(mInstance);

    while (ins->mCluster && !ins->mCluster->EOS()) {

        const mkvparser::BlockEntry* block_entry;
        if (ins->mCluster->GetFirst(block_entry)) break;

        while (block_entry && !block_entry->EOS()) {

            const mkvparser::Block* block = block_entry->GetBlock();

            uint64_t track_num = block->GetTrackNumber();
            const mkvparser::Track* track = ins->mSegment->GetTracks()->GetTrackByNumber(track_num);

            for (int i = 0; i < block->GetFrameCount(); ++i) {

                const mkvparser::Block::Frame& frame = block->GetFrame(i);

                std::vector<uint8_t> buf(frame.len);
                if (ins->mReader.Read(frame.pos, frame.len, buf.data())) continue;

                //////////////////////////////////////
                if (track == ins->mVideoTrack) {

                    Dav1dData data;
                    dav1d_data_wrap(&data, buf.data(), buf.size(), nullptr, nullptr);

                    if (dav1d_send_data(ins->mDav1dContext, &data) == 0) {
                        Dav1dPicture pic;
                        if (dav1d_get_picture(ins->mDav1dContext, &pic) == 0) {
                            Utility::printLog("video frame: (%d) x (%d)",pic.p.w ,pic.p.h);
                            dav1d_picture_unref(&pic);
                        }
                    }
                }

                //////////////////////////////////////
                if (track == ins->mAudioTrack) {

                    std::vector<int16_t> pcm(960 * 2);
                    int samples = opus_decode(ins->mOpusDecoder,
                        buf.data(),
                        buf.size(),
                        pcm.data(),
                        960,
                        0);

                    if (samples > 0) {
                        Utility::printLog("audio samples: %d", samples);
                    }
                }
            }

            if (ins->mCluster->GetNext(block_entry, block_entry)) break;
        }

        ins->mCluster = ins->mSegment->GetNext(ins->mCluster);
    }

    return true;
}

VideoFileHolder::~VideoFileHolder()
{
    VideoFileHolderInstance* ins = static_cast<VideoFileHolderInstance*>(mInstance);

    opus_decoder_destroy(ins->mOpusDecoder);
    dav1d_close(&ins->mDav1dContext);
    ins->mReader.Close();

    delete ins;
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
