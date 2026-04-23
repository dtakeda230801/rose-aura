#include "MediaUtility.h"

#include "WaveFileHolderInternal.h"
#include "OpusFileHolderInternal.h"
#include "VideoFileHolderInternal.h"
#include "MultiBlockBufferInternal.h"
#include "PreRenderThreadInternal.h"
#include "MovieRendererInternal.h"

using namespace RoseAuraMediaUtility;

////////////////////////////////////////////////////
////////////////////////////////////////////////////
#define WFH_INSTANCE static_cast<WaveFileHolderInternal*>(mImpl)

WaveFileHolder::WaveFileHolder(const char* path) :
    mImpl(static_cast<void*>(new WaveFileHolderInternal(path)))
{
}

float* WaveFileHolder::getCurrentFramePointer()
{
    return WFH_INSTANCE->getCurrentFramePointer();
}

void WaveFileHolder::moveCurrentFramePointer(uint32_t frameLen)
{
    WFH_INSTANCE->moveCurrentFramePointer(frameLen);
}

uint32_t WaveFileHolder::getFrameLen()
{
    return WFH_INSTANCE->getFrameLen();
}

uint32_t WaveFileHolder::getRemainFrameLen()
{
    return WFH_INSTANCE->getRemainFrameLen();
}


uint32_t WaveFileHolder::getSamplingRate()
{
    return WFH_INSTANCE->getSamplingRate();
}

uint32_t WaveFileHolder::getChannelNum()
{
    return WFH_INSTANCE->getChannelNum();
}

void WaveFileHolder::reset()
{
    WFH_INSTANCE->reset();
}

WaveFileHolder::~WaveFileHolder()
{
    delete WFH_INSTANCE;
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
#define OFH_INSTANCE static_cast<OpusFileHolderInternal*>(mImpl)

OpusFileHolder::OpusFileHolder(const char* path) :
    mImpl(static_cast<void*>(new OpusFileHolderInternal(path)))
{
}

bool OpusFileHolder::decode()
{
    return OFH_INSTANCE->decode();
}

void OpusFileHolder::getCurrentPointer(float*& data, uint32_t* frameLen)
{
    OFH_INSTANCE->getCurrentPointer(data, frameLen);
}

void OpusFileHolder::moveReadPointer(uint32_t frameLen)
{
    OFH_INSTANCE->moveReadPointer(frameLen);
}

uint32_t OpusFileHolder::getChannels()
{
    return OFH_INSTANCE->getChannels();
}

void OpusFileHolder::setJumpPoint(uint64_t point, uint64_t to)
{
    OFH_INSTANCE->setJumpPoint(point, to);
}


void OpusFileHolder::reset()
{
    OFH_INSTANCE->reset();
}

OpusFileHolder::~OpusFileHolder()
{
    delete OFH_INSTANCE;
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
#define VFH_INSTANCE static_cast<VideoFileHolderInternal*>(mImpl)

VideoFileHolder::VideoFileHolder(const char* path) :
    mImpl(static_cast<void*>(new VideoFileHolderInternal(path)))
{
}

VideoFileHolder::DecoderReturnCode VideoFileHolder::decode()
{
    return VFH_INSTANCE->decode();
}

bool VideoFileHolder::getAudioFrame(float** buff, uint32_t* returnFrameLen, uint32_t requestFrameLen)
{
    return VFH_INSTANCE->getAudioFrame(buff, returnFrameLen, requestFrameLen);
}

bool VideoFileHolder::getVideoFrame(VideoFrame& videoFrame)
{
    return VFH_INSTANCE->getVideoFrame(videoFrame);
}

void VideoFileHolder::releaseVideoFrame(VideoFrame& frame)
{
    VFH_INSTANCE->releaseVideoFrame(frame);
}

uint32_t VideoFileHolder::getSamplingRate()
{
    return VFH_INSTANCE->getSamplingRate();

}
uint32_t VideoFileHolder::getChannels()
{
    return VFH_INSTANCE->getChannels();
}

VideoFileHolder::~VideoFileHolder()
{
    delete VFH_INSTANCE;
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
#define PRT_INSTANCE static_cast<PreRenderThreadInternal*>(mImpl)


bool PreRenderThread::start()
{
    return PRT_INSTANCE->start();
}

void PreRenderThread::wakeUp()
{
    PRT_INSTANCE->wakeUp();
}

void PreRenderThread::finish()
{
    PRT_INSTANCE->finish();
}

void PreRenderThread::finishSelf()
{
    PRT_INSTANCE->finishSelf();
}


PreRenderThread::PreRenderThread() :
    mImpl(static_cast<void*>(new PreRenderThreadInternal(this)))
{
}

PreRenderThread::~PreRenderThread()
{
    delete PRT_INSTANCE;
}


////////////////////////////////////////////////////
////////////////////////////////////////////////////
#define MBB_INSTANCE static_cast<MultiBlockBufferInternal*>(mImpl)

MultiBlockBuffer::MultiBlockBuffer(uint32_t numberOfBlocks, uint32_t framePerBlock, uint32_t elmPerFrame) :
    mImpl(nullptr)
{
    mImpl = static_cast<void*>(new MultiBlockBufferInternal(numberOfBlocks, framePerBlock, elmPerFrame));
}

void MultiBlockBuffer::getWriteBuffer(float*& buffer, uint32_t& aveilFrameLen)
{
    MBB_INSTANCE->getWriteBuffer(buffer, aveilFrameLen);

}

void MultiBlockBuffer::getReadBuffer(float*& buffer, uint32_t& aveilFrameLen)
{
    MBB_INSTANCE->getReadBuffer(buffer, aveilFrameLen);
}

bool MultiBlockBuffer::updateWriteBuffer(uint32_t writeFrameLen, uint64_t* attribute)
{
    return MBB_INSTANCE->updateWriteBuffer(writeFrameLen, attribute);
}

bool MultiBlockBuffer::updateReadBuffer(uint32_t readFrameLen, uint64_t* attribute)
{
    return MBB_INSTANCE->updateReadBuffer(readFrameLen, attribute);
}

uint32_t MultiBlockBuffer::getAvailBlockNum()
{
    return MBB_INSTANCE->getAvailBlockNum();
}

MultiBlockBuffer::~MultiBlockBuffer()
{
    delete MBB_INSTANCE;
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
#define MVR_INSTANCE static_cast<MovieRendererInternal*>(mImpl)


MovieRenderer::MovieRenderer(RoseAura& ra, const int8_t* movieFile, IMovieRendererCallback* cb) : 
    mImpl(static_cast<void*>(new MovieRendererInternal(ra, movieFile, cb)))
{
}

bool MovieRenderer::start()
{
    return MVR_INSTANCE->start();
}

bool MovieRenderer::stop()
{
    return MVR_INSTANCE->stop();
}

MovieRenderer::~MovieRenderer()
{
    delete MVR_INSTANCE;
}
