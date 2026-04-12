#pragma once

#include <opus/opusfile.h>
#include "mkvparser/mkvparser.h"
#include "mkvparser/mkvreader.h"

extern "C" {
#include <dav1d/dav1d.h>
#include <opus/opus.h>
}

#include "MediaUtility.h"

namespace RoseAuraMediaUtility {
	//////////////////////////////////////
	class VideoFileHolder::VideoFileHolderImpl
	{
	public:
		VideoFileHolderImpl(const char* path);

		DecoderReturnCode decode();

		bool     getAudioFrame(float** buff, unsigned int* returnFrameLen, unsigned int requestFrameLen);
		bool     getVideoFrame(VideoFileHolder::VideoFrame& videoFrame);

		void     releaseVideoFrame(VideoFrame& frame);

		uint32_t getSamplingRate();
		uint32_t getChannels();


		virtual ~VideoFileHolderImpl();
	private:
		std::vector<unsigned char>
			extractSequenceHeaderOBU(const unsigned char* priv, unsigned int size);
		void sendToDav1d(const unsigned char* data, unsigned int size);
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

		unsigned int				 mSamplingRate;
		unsigned int				 mChannels;
		float*						 mAudioBuffer;
		unsigned int				 mAudioFrameLen;
		unsigned int				 mAudioReadPointer;
		bool						 mAudioReady;
	};
}