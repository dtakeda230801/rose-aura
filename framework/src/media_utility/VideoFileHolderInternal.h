#pragma once

#include <cstdint>

#include "mkvparser/mkvparser.h"
#include "mkvparser/mkvreader.h"

extern "C" {
#include <dav1d/dav1d.h>
#include <opus/opus.h>
}

#include "MediaUtility.h"

using namespace RoseAuraMediaUtility;

class VideoFileHolderInternal {
public:
	VideoFileHolderInternal(const char* path);

	VideoFileHolder::DecoderReturnCode decode();

	bool     getAudioFrame(float** buff, uint32_t* returnFrameLen, uint32_t requestFrameLen);
	bool     getVideoFrame(VideoFileHolder::VideoFrame& videoFrame);

	void     releaseVideoFrame(VideoFileHolder::VideoFrame& frame);

	uint32_t getSamplingRate();
	uint32_t getChannels();

	virtual ~VideoFileHolderInternal();
private:
	std::vector<unsigned char>
		extractSequenceHeaderOBU(const uint8_t* priv, uint32_t size);
	void sendToDav1d(const uint8_t* data, uint32_t size);
	VideoFileHolder::VideoFrame
		convertPictureFormat(const Dav1dPicture& pic);
	bool getPicture();

	static constexpr unsigned int AUDIO_BUFFER_FRAME_LEN = 960;

	mkvparser::MkvReader		 mReader;
	OpusDecoder*				 mOpusDecoder;
	mkvparser::Segment*			 mSegment;
	const mkvparser::Cluster*	 mCluster;
	const mkvparser::Track*		 mVideoTrack;
	const mkvparser::Track*		 mAudioTrack;
	Dav1dContext*				 mDav1dContext;
	const mkvparser::BlockEntry* mBlockEntry;
	unsigned int     			 mFrameCount;

	Dav1dPicture				 mPicture;
	bool						 mPictureReady;
	uint64_t					 mLatestTimeStamp;

	unsigned int				 mSamplingRate;
	unsigned int				 mChannels;
	float* mAudioBuffer;
	unsigned int				 mAudioFrameLen;
	unsigned int				 mAudioReadPointer;
	bool						 mAudioReady;
};
