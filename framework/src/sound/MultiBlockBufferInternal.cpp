
#include "MultiBlockBufferInternal.h"
#include "Utility.h"

//#define __BUFFER_DEBUG__

MultiBlockBufferInternal::MultiBlockBufferInternal(uint32_t numberOfBlocks, uint32_t framePerBlock, uint32_t elmPerFrame) :
	  mBufferHolder{}
    , mSelectedWriteBuffer(nullptr)
	, mSelectedReadBuffer(nullptr)
{
	mBufferHolder.mBlock          = new Buffer*[numberOfBlocks];
	mBufferHolder.mNumberOfBlocks = numberOfBlocks;
	mBufferHolder.mElmPerFrame    = elmPerFrame;
	mBufferHolder.mBlockWP        = 0;
	mBufferHolder.mBlockRP        = 0;

	for (uint32_t i = 0; i < mBufferHolder.mNumberOfBlocks; ++i) {
		mBufferHolder.mBlock[i] = new Buffer();
		mBufferHolder.mBlock[i]->mBufferSize     = framePerBlock * elmPerFrame;
		mBufferHolder.mBlock[i]->mBuffer         = new float[mBufferHolder.mBlock[i]->mBufferSize];
		mBufferHolder.mBlock[i]->mWP             = 0;
		mBufferHolder.mBlock[i]->mRP             = 0;
		mBufferHolder.mBlock[i]->mAttribute      = 0;
		mBufferHolder.mBlock[i]->mWriteAvailable = true;
	}
}

void MultiBlockBufferInternal::getWriteBuffer(float*& buffer, uint32_t& aveilFrameLen)
{
#ifdef __BUFFER_DEBUG__
	Utility::printLog(">>> getWriteBuffer");
	dumpBudderStatus();
#endif

	buffer        = nullptr;
	aveilFrameLen = 0;

	Buffer* selectedBuffer = mBufferHolder.mBlock[mBufferHolder.mBlockWP];

	mMutex.lock();
	if (selectedBuffer->mWriteAvailable) {
		mSelectedWriteBuffer = selectedBuffer;
	} else {
		mSelectedWriteBuffer = nullptr;
	}
	mMutex.unlock();

	if (mSelectedWriteBuffer) {
		buffer        = selectedBuffer->mBuffer;
		aveilFrameLen = selectedBuffer->mBufferSize / mBufferHolder.mElmPerFrame;
	}

	return;
}

void MultiBlockBufferInternal::getReadBuffer(float*& buffer, uint32_t& aveilFrameLen)
{
#ifdef __BUFFER_DEBUG__
	Utility::printLog(">>> getReadBuffer");
	dumpBudderStatus();
#endif
	buffer        = nullptr;
	aveilFrameLen = 0;

	Buffer* selectedBuffer = mBufferHolder.mBlock[mBufferHolder.mBlockRP];

	mMutex.lock();
	if (!selectedBuffer->mWriteAvailable) {
		mSelectedReadBuffer = selectedBuffer;
	} else {
		mSelectedReadBuffer = nullptr;
	}
	mMutex.unlock();

	if (mSelectedReadBuffer) {
		buffer = selectedBuffer->mBuffer + selectedBuffer->mRP;
		aveilFrameLen = (selectedBuffer->mWP - selectedBuffer->mRP) / mBufferHolder.mElmPerFrame;
	}

	return;
}

bool MultiBlockBufferInternal::updateWriteBuffer(uint32_t writeFrameLen, uint64_t* attribute)
{
	if (!mSelectedWriteBuffer) {
		return false;
	}

	if (writeFrameLen == 0) {
		return false;
	}

	mMutex.lock();

	mSelectedWriteBuffer->mWP = writeFrameLen * mBufferHolder.mElmPerFrame;
	mSelectedWriteBuffer->mWriteAvailable = false;

	if (attribute) {
		mSelectedWriteBuffer->mAttribute = *attribute;
	}
	mSelectedWriteBuffer = nullptr;

	++mBufferHolder.mBlockWP;
	if (mBufferHolder.mBlockWP == mBufferHolder.mNumberOfBlocks) {
		mBufferHolder.mBlockWP = 0;
	}

	mMutex.unlock();

#ifdef __BUFFER_DEBUG__
	Utility::printLog(">>> updateWriteBuffer");
	dumpBudderStatus();
#endif
	return true;
}

bool MultiBlockBufferInternal::updateReadBuffer(uint32_t readFrameLen, uint64_t* attribute)
{
	if (!mSelectedReadBuffer) {
		return false;
	}

	mMutex.lock();

	mSelectedReadBuffer->mRP += readFrameLen * mBufferHolder.mElmPerFrame;

	if (attribute) {
		*attribute = mSelectedReadBuffer->mAttribute;
	}

	if (mSelectedReadBuffer->mRP == mSelectedReadBuffer->mWP) {
		mSelectedReadBuffer->mWriteAvailable = true;
		mSelectedReadBuffer->mRP = 0;
		mSelectedReadBuffer->mWP = 0;
		mSelectedReadBuffer = nullptr;

		++mBufferHolder.mBlockRP;
		if (mBufferHolder.mBlockRP == mBufferHolder.mNumberOfBlocks) {
			mBufferHolder.mBlockRP = 0;
		}
	}
	mMutex.unlock();

#ifdef __BUFFER_DEBUG__
	Utility::printLog(">>> updateReadBuffer");
	dumpBudderStatus();
#endif
	return true;
}

MultiBlockBufferInternal::~MultiBlockBufferInternal()
{
	for (uint32_t i = 0; i < mBufferHolder.mNumberOfBlocks; ++i) {
		delete[] mBufferHolder.mBlock[i]->mBuffer;
		delete   mBufferHolder.mBlock[i];
	}
	delete[] mBufferHolder.mBlock;
}

void MultiBlockBufferInternal::dumpBudderStatus()
{
	Utility::printLog("=================");
	Utility::printLog("BufferHolder : wp(%d) rp(%d)", mBufferHolder.mBlockWP, mBufferHolder.mBlockRP);
	Utility::printLog("Block[0] : wp(%d) rp(%d) wa(%s)", mBufferHolder.mBlock[0]->mWP
		                                               , mBufferHolder.mBlock[0]->mRP
	                                                   , mBufferHolder.mBlock[0]->mWriteAvailable ? "true" : "false");
	Utility::printLog("Block[1] : wp(%d) rp(%d) wa(%s)", mBufferHolder.mBlock[1]->mWP
													   , mBufferHolder.mBlock[1]->mRP
													   , mBufferHolder.mBlock[1]->mWriteAvailable ? "true" : "false");
	Utility::printLog("=================");
}
