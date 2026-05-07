#include <cmath>

#include "MathematicalUtility.h"

float MathematicalUtility::calcDistance(IWorldNavigator::Vec3& a, IWorldNavigator::Vec3& b)
{
	return static_cast<float>(std::sqrt((static_cast<double>(b.mX - a.mX) * static_cast<double>(b.mX - a.mX))
		+ (static_cast<double>(b.mY - a.mY) * static_cast<double>(b.mY - a.mY))
		+ (static_cast<double>(b.mZ - a.mZ) * static_cast<double>(b.mZ - a.mZ))));

}
