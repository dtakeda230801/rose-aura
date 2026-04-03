#pragma once

#include <vector>
#include <thread>

#include "IMediaCoordinator.h"

class MediaCoordinator : public IMediaCoordinator {
public:
	//////////////////////////////////////////////////////////
	// Internal Classes
	//////////////////////////////////////////////////////////

	struct WavData {
		int channels;
		int sampleRate;
		std::vector<float> samples;
		unsigned int current;
	};


	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	RARetCode start();
	RARetCode stop();

	void test();

	MediaCoordinator();
	virtual ~MediaCoordinator() = default;

private:
	bool loadWav(const char* path, WavData& out);

	unsigned int requestData(float** buff,unsigned int size, unsigned int chs);

	void renderToDevice();

	WavData			mWavData;

	std::thread		mThread;

	bool			mStarted;

	bool			mPlay;

};