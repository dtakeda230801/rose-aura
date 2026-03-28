#pragma once

#include "RoseAuraReturnCode.h"

using namespace RoseAuraReturnCode;

class IGraphicsManager {
public:
	class IObjectRenderer {
	public:
		virtual void render() = 0;

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
};