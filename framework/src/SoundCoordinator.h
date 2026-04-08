#pragma once

#include <vector>
#include <thread>

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

    unsigned int getSystemSamplingRate();
    unsigned int getSystemChannels();

    RARetCode registerRenderer(ISoundRenderer* renderer);

    void recover();

	SoundCoordinator();
	virtual ~SoundCoordinator();

private:
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
        float*         mBuffer;
        unsigned int   mBufferSize;
        unsigned int   mWritePointer;
        unsigned int   mReadPointer;
    };

    struct SBHolder {
        SoundBuffer     mBuffer[2];
        unsigned int    mWritePointer;
        unsigned int    mReadPointer;
    };

    class DataWriter : public IDataWriter {
    public:
        virtual RARetCode write(float* buff, unsigned int frameLen);

        void setBuffer(float* buff, unsigned int size);
        void reset();

        DataWriter();
        virtual ~DataWriter() = default;

    private:
        float*          mWriteBuffer;
        unsigned int    mWriteBufferSize;
        unsigned int    mWroteFrame;
    };


    void         renderToDevice();
    unsigned int requestDataInternal(float** buff, unsigned int frameNum);
    void         allocateSystemBuffer();
    void         releaseSystemBuffer();
    void         makeChannelOffsetMap(unsigned int mask);
    void         dumpSystemBufferCondition();


	std::thread		mThread;
	bool			mStarted;
    bool			mRecover;

	unsigned int	mSystemChannels;
	unsigned int	mSystemSamplingRate;

    static constexpr unsigned int SYSTEM_BUFFER_DURATION  = 200000; // 20 msec
    static constexpr unsigned int SYSTEM_BUFFER_BASE_SIZE = 480;

    static constexpr unsigned int SC_SAMPLING_RATE = 48000;
    static constexpr unsigned int SC_CHANNEL       = 2;

    SBHolder       mSystemBuffer;

    unsigned int   mChOffsetMap[2];

    SamplingRateConverter
                   mSystemSrc;

    float*         mSystemSRCOutBuff;
    unsigned int   mSystemSRCOutFrameCurrent;
    unsigned int   mSystemSRCOutFrameLen;

    std::vector<ISoundRenderer*>
                   mSoundData;


    DataWriter     mDataWriter;
};