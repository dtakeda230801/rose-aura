#pragma once

class SamplingRateConverter {
public:
	void reset();
	void apply(float* src, unsigned int srcFrameLen, float** dst, unsigned int* dstFrameLen);
	void releaseBuffer();
	void setConfig(unsigned int srcFs, unsigned int srcCh, unsigned int dstFs);

	SamplingRateConverter();
	virtual ~SamplingRateConverter();
private:
	float			mSrePeriod;
	unsigned int	mSrcCh;
	float			mDstPeriod;
	float*			mOverwrap;
	float*			mDstBuffer;

	unsigned long	mSrcTimePointer;
	unsigned long   mDstTimePointer;
};
