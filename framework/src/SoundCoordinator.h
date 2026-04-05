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
    unsigned int getChannels();

    RARetCode playOneShut(RequestDataFunction func);

    void recover();

	void test();

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

    void         renderToDevice();
    unsigned int requestData(float** buff,unsigned int frameNum);
    unsigned int requestDataInternal(float** buff, unsigned int frameNum);
    void         allocateSystemBuffer();
    void         releaseSystemBuffer();
    void         dumpSystemBufferCondition();


	std::thread		mThread;
	bool			mStarted;
    bool			mRecover;

	unsigned int	mSystemChannels;
	unsigned int	mSystemSamplingRate;

	bool			mPlay;
	Utility::WaveFileHolder*
					mWaveFileHolder;

	SamplingRateConverter
					mSamplingRateConverter;

	float*          mSRCOutBuff;
	unsigned int    mSRCOutFrameCurrent;
	unsigned int    mSRCOutFrameLen;

	const unsigned int SYSTEM_BUFFER_DURATION  = 200000; // 20 msec
    const unsigned int SYSTEM_BUFFER_BASE_SIZE = 480;

    SBHolder       mSystemBuffer;

    std::vector<RequestDataFunction>
                   mRequestDataFuncs;

};