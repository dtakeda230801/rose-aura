#pragma once

#include <string>

#include "RoseAuraReturnCode.h"

using namespace RoseAuraReturnCode;

#define SHADER_POINTER(x) static_cast<Shader*>(x)

class IGraphicsManager {
public:
	struct Conf {
		uint32_t		mWindowWidth;
		uint32_t		mWindowHeight;
		uint32_t		mFrameRate;
		const char*		mWindowTitle;
	};

	class IGraphicsRenderer {
	public:
		virtual void preprocess() = 0;
		virtual void render()     = 0;

		virtual ~IGraphicsRenderer() = default;
	protected:
		IGraphicsRenderer() = default;
	};

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	virtual void      runUntilClosed(Conf conf)					= 0;
	virtual RARetCode setRenderer(IGraphicsRenderer* renderer)	= 0;
	virtual RARetCode removeRenderer(IGraphicsRenderer* renderer)	= 0;
	virtual RARetCode setShaderFile(std::string file)           = 0;
	virtual void*	  getShader(std::string file)               = 0;
};