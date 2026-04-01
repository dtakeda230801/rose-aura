#pragma once

#include <vector>

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
	};


	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	void test();

	MediaCoordinator() = default;
	virtual ~MediaCoordinator() = default;

private:
	bool loadWav(const char* path, WavData& out);

	void renderAudioThread();


	bool mStarted;

};