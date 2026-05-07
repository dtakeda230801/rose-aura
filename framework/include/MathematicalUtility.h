#pragma once

#include "IWorldNavigator.h"

class MathematicalUtility {
public:

	static float calcDistance(IWorldNavigator::Vec3& a, IWorldNavigator::Vec3& b);

private:
	MathematicalUtility() = default;
	virtual ~MathematicalUtility() = default;
};
