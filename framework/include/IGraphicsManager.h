#pragma once

#include <string>

#include "RoseAuraReturnCode.h"

#define SHADER_POINTER(x) static_cast<Shader*>(x)

class IGraphicsManager {
public:
	using SHADER_ID = uint32_t;
	using FONT_ID   = uint32_t;

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
	virtual void      runUntilClosed(Conf conf)					    = 0;
	virtual void      exit()                                        = 0;
	virtual RARetCode setRenderer(IGraphicsRenderer* renderer)	    = 0;
	virtual RARetCode removeRenderer(IGraphicsRenderer* renderer)	= 0;
	virtual RARetCode setShaderFile(std::string vsfile
		                          , std::string fsfile
		                          , SHADER_ID& id)                  = 0;
	virtual RARetCode setShader(std::string vsString
							  , std::string fsString
							  , SHADER_ID& id)						= 0;
	virtual void*	  getShader(SHADER_ID id)                       = 0;
	virtual RARetCode setFont(std::string file, FONT_ID& id)        = 0;
	virtual void*     getFont(FONT_ID id)                           = 0;
};