#pragma once

#include <cstdint>
#include <vector>
#include <thread>
#include <mutex>

#include "ISoundCoordinator.h"
#include "sound/SamplingRateConverter.h"
#include "media_utility/MultiBlockBufferInternal.h"

#include "Utility.h"

#include <mmdeviceapi.h>

class SoundCoordinator : public ISoundCoordinator {
public:
	//////////////////////////////////////////////////////////
	// Internal Classes
	//////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	RARetCode start();
	RARetCode stop();

    uint32_t getSystemSamplingRate();
    uint32_t getSystemChannels();
    uint32_t getDelayTime();

    RARetCode registerRenderer(ISoundRenderer* renderer);
    RARetCode unregisterRenderer(ISoundRenderer* renderer);

    void recover();

	SoundCoordinator();
	virtual ~SoundCoordinator() = default;

private:
    //////////////////////////////////////////////////////////
    // Private Classes
    //////////////////////////////////////////////////////////
    class SystemNotification :public IMMNotificationClient {
	public:

        ULONG STDMETHODCALLTYPE AddRef() override
        {
            return InterlockedIncrement(&mRefCounter);
        }

        ULONG STDMETHODCALLTYPE Release() override
        {
            ULONG refCounter = InterlockedDecrement(&mRefCounter);
            if (refCounter == 0) {
                delete this;
            }
            return refCounter;
        }

        HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID riid, VOID** ppvInterface) override
        {
            if (riid == __uuidof(IUnknown) || riid == __uuidof(IMMNotificationClient))
            {
                *ppvInterface = (IMMNotificationClient*)this;
                AddRef();
                return S_OK;
            }
            *ppvInterface = nullptr;
            return E_NOINTERFACE;
        }

        HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(
            EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId) override
        {
            if (flow == eRender && role == eConsole)
            {
                mSoundCoordinator.recover();
            }
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR) override { return S_OK; }
        HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR) override { return S_OK; }
        HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR, DWORD) override { return S_OK; }
        HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR, const PROPERTYKEY) override { return S_OK; }

        SystemNotification(SoundCoordinator& soundCoordinator) : mSoundCoordinator(soundCoordinator)
        {};

		virtual ~SystemNotification() = default;
    private:
        LONG                mRefCounter = 1;
        SoundCoordinator&   mSoundCoordinator;
	};

    //////////////////////////////////////////////////////////
    class DataWriter : public IDataWriter {
    public:
        virtual RARetCode write(float* buff, uint32_t frameLen);

        void setBuffer(float*& buffer, uint32_t availFrames);
        void reset();

        DataWriter();
        virtual ~DataWriter() = default;

    private:
        uint32_t    mWroteFrame;
        uint32_t    mWriteAvailFrames;
        float*      mWriteBuffer;
    };

    //////////////////////////////////////////////////////////
    // Private Methods
    //////////////////////////////////////////////////////////
    void     renderToDevice();
    uint32_t requestDataInternal(float** buff, uint32_t frameNum);
    void     makeChannelOffsetMap(uint32_t mask);
    void     calcAverageDelayTime(uint64_t sample);

    //////////////////////////////////////////////////////////
    // Private Members
    //////////////////////////////////////////////////////////
    static constexpr uint32_t SYSTEM_BUFFER_DURATION = 200000; // 20 msec

    uint32_t	    mSystemChannels;
    uint32_t	    mSystemSamplingRate;

    static constexpr uint32_t SC_BUFFER_BASE_SIZE = 240;
    static constexpr uint32_t SC_BUFFER_BLOCK_NUM = 2;
    static constexpr uint32_t SC_SAMPLING_RATE    = 48000;
    static constexpr uint32_t SC_CHANNEL          = 2;

    std::thread		mThread;
	bool			mStarted;
    bool			mRecover;
    std::mutex      mMutex;

    float           mAverageDelayTime;

    uint32_t        mChOffsetMap[2];

    std::vector<ISoundRenderer*>
                    mRenderers;

    DataWriter      mDataWriter;

    std::unique_ptr<MultiBlockBufferInternal>
                    mSCBuffer;

    SamplingRateConverter
                    mSCSrc;

    float*          mSCSRCOutBuff;
    uint32_t        mSCSRCOutFrameCurrent;
    uint32_t        mSCSRCOutFrameLen;

};