#pragma once

#include <cstdint>
#include <atomic>
#include <mutex>

class MultiBlockBufferInternal
{
public:
	MultiBlockBufferInternal(uint32_t numberOfBlocks, uint32_t framePerBlock, uint32_t elmPerFrame);

	void getWriteBuffer(float*& buffer, uint32_t& aveilFrameLen);
	void getReadBuffer(float*& buffer, uint32_t& aveilFrameLen);
	bool updateWriteBuffer(uint32_t writeFrameLen, uint64_t* attribute);
    bool updateReadBuffer(uint32_t readFrameLen, uint64_t* attribute);

	virtual ~MultiBlockBufferInternal();

private:
    void dumpBudderStatus();

    struct Buffer {
        float*              mBuffer;
        uint32_t            mBufferSize;
        uint32_t            mWP;
        uint32_t            mRP;
        uint64_t            mAttribute;
        bool                mWriteAvailable;
    };

    struct BufferHolder {
        Buffer**    mBlock;
        uint32_t    mNumberOfBlocks;
        uint32_t    mElmPerFrame;
        uint32_t    mBlockWP;
        uint32_t    mBlockRP;
    };

    BufferHolder    mBufferHolder;
    std::mutex      mMutex;
    Buffer*         mSelectedWriteBuffer;
    Buffer*         mSelectedReadBuffer;
};
