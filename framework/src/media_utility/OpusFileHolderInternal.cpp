#include <opus/opusfile.h>

#include "OpusFileHolderInternal.h"
#include "Utility.h"


OpusFileHolderInternal::OpusFileHolderInternal(const char* path) :
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

    mFile = static_cast<void*>(file);
    mChannels = static_cast<uint32_t>(ret);
    mBuffer = new float[BUFF_FRAME_LEN * mChannels];
}

bool OpusFileHolderInternal::decode()
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
                    mFrameLen -= static_cast<uint32_t>(mFrameCounter - mJumpPoint);

                    uint64_t seekPoint;
                    uint32_t preDecodeLen;
                    if (mJumpTo < BUFF_FRAME_LEN) {
                        seekPoint = 0;
                        preDecodeLen = static_cast<uint32_t>(mJumpTo);
                    }
                    else {
                        seekPoint = mJumpTo - BUFF_FRAME_LEN;
                        preDecodeLen = BUFF_FRAME_LEN;
                    }
                    op_pcm_seek(file, seekPoint);

                    uint32_t preDecodeOut = 0;
                    float* preDecodeBuffer = new float[preDecodeLen * mChannels];
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

void OpusFileHolderInternal::getCurrentPointer(float*& data, uint32_t* frameLen)
{

    data = mBuffer + mReadPointer * mChannels;
    *frameLen = mFrameLen;
}

void OpusFileHolderInternal::moveReadPointer(uint32_t frameLen)
{
    mReadPointer += frameLen;
    if (mReadPointer >= mFrameLen) {
        mFrameLen = 0;
        mReadPointer = 0;
    }
}

uint32_t OpusFileHolderInternal::getChannels()
{
    return mChannels;
}

void OpusFileHolderInternal::setJumpPoint(uint64_t point, uint64_t to)
{
    mJump = true;
    mJumpPoint = point;
    mJumpTo = to;
}


void OpusFileHolderInternal::reset()
{
    OggOpusFile* file = static_cast<OggOpusFile*>(mFile);

    op_pcm_seek(file, 0);
    mFrameLen = 0;
    mReadPointer = 0;
    mNoFinish = true;
    mJump = false;
    mJumpTo = 0;
    mJumpPoint = 0;
    mFrameCounter = 0;
}

OpusFileHolderInternal::~OpusFileHolderInternal()
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
