#pragma once

#include <string>

#include "RoseAuraReturnCode.h"

using namespace RoseAuraReturnCode;

#define SHADER_POINTER(x) static_cast<Shader*>(x)

class IGraphicsManager {
public:
	class IObjectRenderer {
	public:
		virtual void doPreprocess() = 0;
		virtual void render()       = 0;

		virtual ~IObjectRenderer() = default;
	protected:
		IObjectRenderer() = default;
	};

	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	virtual void      runUntilClosed()							= 0;
	virtual RARetCode setRenderer(IObjectRenderer* renderer)	= 0;
	virtual RARetCode removeRenderer(IObjectRenderer* renderer)	= 0;
	virtual RARetCode setShaderFile(std::string file)           = 0;
	virtual void*	  getShader(std::string file)               = 0;
};