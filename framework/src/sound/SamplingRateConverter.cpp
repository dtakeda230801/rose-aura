#include "SamplingRateConverter.h"


void SamplingRateConverter::reset()
{
	mSrePeriod = 1.0f / 44100.0f;
	mSrcCh     = 2;
	mDstPeriod = 1.0f / 48000.0f;

	if (mOverwrap) {
		delete mOverwrap;
	}
	mOverwrap  = new float[mSrcCh];
	for (unsigned int i = 0; i < mSrcCh; i++) {
		mOverwrap[i] = 0;
	}

	mSrcTimePointer = 0;
	mDstTimePointer = 0;
}

void SamplingRateConverter::apply(float*        src
	                            , unsigned int  srcFrameLen
	                            , float**       dst
							    , unsigned int* dstFrameLen)
{
	unsigned int dstSampleLen = static_cast<unsigned int>(
							(float)(srcFrameLen + 1) * (float)mSrePeriod / (float)mDstPeriod);

	mDstBuffer  = new float[ dstSampleLen * mSrcCh ];
	*dst        = mDstBuffer;
	*dstFrameLen = 0;

	float* src_current  = mOverwrap;
	float* src_plus_one = src;
	float* dst_current  = *dst;

	for (unsigned int i = 0; i < srcFrameLen; i++) {
		float t_src          = (float)mSrcTimePointer       * mSrePeriod;
		float t_src_plus_one = (float)(mSrcTimePointer + 1) * mSrePeriod;

		while (true) {
			float t_dst = (float)mDstTimePointer * mDstPeriod;
			if (t_src <= t_dst && t_dst < t_src_plus_one) {
				for (unsigned int ch = 0; ch < mSrcCh; ch++) {
					*dst_current++ = (*(src_plus_one + ch) - *(src_current + ch)) * (t_dst - t_src) + *(src_current + ch);
				}
				(*dstFrameLen)++;
				mDstTimePointer++;
			}
			else {
				break;
			}
		}

		mSrcTimePointer++;
		src_current   = src_plus_one;
		src_plus_one += mSrcCh;
	}

	for (unsigned int i = 0; i < mSrcCh; i++){
		mOverwrap[i] = *(src_current + i);
	}
}

void SamplingRateConverter::releaseBuffer()
{
	if (mDstBuffer)
	{
		delete[] mDstBuffer;
		mDstBuffer = nullptr;
	}
}

void SamplingRateConverter::setConfig(unsigned int srcFs, unsigned int srcCh, unsigned int dstFs)
{
	mSrePeriod = 1.0f / (float)srcFs;
	mSrcCh     = srcCh;
	mDstPeriod = 1.0f / (float)dstFs;
}

SamplingRateConverter::SamplingRateConverter() :
	  mSrePeriod(1.0f/44100.0f)
	, mSrcCh(2)
	, mDstPeriod(1.0f/48000.0f)
	, mDstBuffer(nullptr)
	, mSrcTimePointer(0)
	, mDstTimePointer(0)
{
	mOverwrap = new float[mSrcCh];
	for (unsigned int i = 0; i < mSrcCh; i++) {
		mOverwrap[i] = 0;
	}
}

SamplingRateConverter::~SamplingRateConverter()
{
	releaseBuffer();
	if (mOverwrap)
	{
		delete mOverwrap;
		mOverwrap = nullptr;
	}

}
