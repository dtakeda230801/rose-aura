#pragma once

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
	virtual void runUntilClosed()							= 0;
	virtual void setRenderer(IObjectRenderer* renderer)		= 0;
	virtual void removeRenderer(IObjectRenderer* renderer)	= 0;
};