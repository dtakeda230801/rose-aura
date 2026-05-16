#pragma once

#include <string>

#include "RoseAuraReturnCode.h"

#define SHADER_POINTER(x) static_cast<Shader*>(x)

class IGraphicsManager {
public:
	using SHADER_ID = uint32_t;
	using FONT_ID   = uint32_t;
	using MODEL_ID  = uint32_t;

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

	class IModelWrapper {
	public:
		virtual void*     getModel()                      = 0;
		virtual void*     getAnimetionModel()             = 0;
		virtual uint32_t  getAnimationNum()               = 0;
		virtual RARetCode selectAnimation(uint32_t index) = 0;
		virtual void      setAdjustment(uint32_t startOffset
			                          , uint32_t endOffset
			                          , float    rate)    = 0;
	};


	//////////////////////////////////////////////////////////
	// APIs
	//////////////////////////////////////////////////////////
	virtual void      runUntilClosed(Conf conf)					    = 0;
	virtual void      exit()                                        = 0;
	virtual RARetCode setRenderer(IGraphicsRenderer* renderer)	    = 0;
	virtual RARetCode removeRenderer(IGraphicsRenderer* renderer)	= 0;

	virtual SHADER_ID setShaderFile(std::string vsFile
		                          , std::string fsFile)             = 0;
	virtual SHADER_ID setShader(std::string     vsStr
		                      , std::string     fsStr)              = 0;
	virtual void*	  getShader(SHADER_ID id)                       = 0;
	virtual RARetCode removeShader(SHADER_ID id)					= 0;


	virtual FONT_ID   setFont(std::string file)                     = 0;
	virtual void*     getFont(FONT_ID id)                           = 0;
	virtual RARetCode removeFont(FONT_ID id)                        = 0;

	virtual MODEL_ID  setModel(std::string file
		                               , bool        loadAnimation) = 0;
	virtual IModelWrapper* 
		              getModelWrapper(MODEL_ID id)                  = 0;
	virtual void      releaseModelWrapper(MODEL_ID id)              = 0;
};