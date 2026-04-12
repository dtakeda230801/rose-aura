#pragma once

#include <cstdint>
#include <vector>
#include <thread>
#include <mutex>

#include "ISoundCoordinator.h"
#include "sound/SamplingRateConverter.h"
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
	virtual ~SoundCoordinator();

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

    struct SoundBuffer {
        float*     mBuffer;
        uint32_t   mBufferSize;
        uint32_t   mWritePointer;
        uint32_t   mReadPointer;
        uint64_t   mWriteTime;
    };

    struct SBHolder {
        SoundBuffer mBuffer[2];
        uint32_t    mWritePointer;
        uint32_t    mReadPointer;
    };

    //////////////////////////////////////////////////////////
    class DataWriter : public IDataWriter {
    public:
        virtual RARetCode write(float* buff, uint32_t frameLen);

        void setBuffer(SoundBuffer* soundBuffer);
        void reset();

        DataWriter();
        virtual ~DataWriter() = default;

    private:
        uint32_t        mWroteFrame;
        SoundBuffer*    mSoundBuffer;
    };

    //////////////////////////////////////////////////////////
    // Private Methods
    //////////////////////////////////////////////////////////
    void     renderToDevice();
    uint32_t requestDataInternal(float** buff, uint32_t frameNum);
    void     allocateSystemBuffer();
    void     releaseSystemBuffer();
    void     makeChannelOffsetMap(uint32_t mask);
    void     calcAverageDelayTime(uint64_t sample);
    void     dumpSystemBufferCondition();

    //////////////////////////////////////////////////////////
    // Private Members
    //////////////////////////////////////////////////////////
    static constexpr uint32_t SYSTEM_BUFFER_DURATION = 200000; // 20 msec
    static constexpr uint32_t SYSTEM_BUFFER_BASE_SIZE = 480;

    static constexpr uint32_t SC_SAMPLING_RATE = 48000;
    static constexpr uint32_t SC_CHANNEL = 2;

    std::thread		mThread;
	bool			mStarted;
    bool			mRecover;
    std::mutex      mMutex;

	uint32_t	    mSystemChannels;
	uint32_t	    mSystemSamplingRate;
    float           mAverageDelayTime;

    SBHolder        mSystemBuffer;

    uint32_t        mChOffsetMap[2];

    std::vector<ISoundRenderer*>
        mRenderers;

    DataWriter      mDataWriter;

    SamplingRateConverter
                    mSystemSrc;

    float*          mSystemSRCOutBuff;
    uint32_t        mSystemSRCOutFrameCurrent;
    uint32_t        mSystemSRCOutFrameLen;

};