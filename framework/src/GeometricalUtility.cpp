#include <cmath>

#include "GeometricalUtility.h"

//////////////////////////////////////
struct IntVec3 {
	float mX;
	float mY;
	float mZ;
};

IntVec3 conv(const IWorldNavigator::Vec3& in)
{
	return IntVec3{ (float)(in.mX), (float)(in.mY), (float)(in.mZ) };
}

IntVec3 add(IntVec3& a, IntVec3& b)
{
	return IntVec3{a.mX + b.mX, a.mY + b.mY, a.mZ + b.mZ};
}

IntVec3 subtract(IntVec3& a, IntVec3& b)
{
	return IntVec3{ a.mX - b.mX, a.mY - b.mY, a.mZ - b.mZ };
}

IntVec3 multiply(float a, IntVec3& b)
{
	return IntVec3{ a * b.mX, a * b.mY, a * b.mZ };
}

IntVec3 cross(IntVec3& a, IntVec3& b)
{
	float x = a.mY * b.mZ - a.mZ * b.mY;
	float y = a.mZ * b.mX - a.mX * b.mZ;
	float z = a.mX * b.mY - a.mY * b.mX;

	return IntVec3{ x, y, z};
}

float dot(IntVec3& a, IntVec3& b)
{
	return a.mX * b.mX + a.mY * b.mY + a.mZ * b.mZ;
}

//////////////////////////////////////
float GeometricalUtility::calcDistance(IWorldNavigator::Vec3& a, IWorldNavigator::Vec3& b)
{
	return static_cast<float>(std::sqrt((static_cast<double>(b.mX - a.mX) * static_cast<double>(b.mX - a.mX))
		+ (static_cast<double>(b.mY - a.mY) * static_cast<double>(b.mY - a.mY))
		+ (static_cast<double>(b.mZ - a.mZ) * static_cast<double>(b.mZ - a.mZ))));

}

//////////////////////////////////////
bool GeometricalUtility::detectCollision(const Vertex& vertex
	                                   , const Indices& indeices
	                                   , const IWorldNavigator::Vec3& start
	                                   , const IWorldNavigator::Vec3& end
									   , IWorldNavigator::Vec3& intersection)
{
	IntVec3 s = conv(start);
	IntVec3 e = conv(end);

	for (uint32_t i = 0; i < indeices.size(); i += 3) {
		IntVec3 a = conv(vertex[indeices[i]]);
		IntVec3 b = conv(vertex[indeices[i+1]]);
		IntVec3 c = conv(vertex[indeices[i+2]]);


		IntVec3 ab = subtract(b, a);
		IntVec3 ac = subtract(c, a);

		IntVec3 bc = subtract(c, b);
		IntVec3 ca = subtract(a, c);

		IntVec3 n  = cross(ab, ac);

		IntVec3 se = subtract(e, s);

		float d = dot(n, se);

		if (d == 0.0f) {
			continue;
		}

		IntVec3 as = subtract(s, a);

		float m = -1.0 * dot(n, as);

		float t = m / d;

		if (t < 0 || 1 < t) {
			continue;
		}

		IntVec3 mse = multiply(t, se);

		IntVec3 p = add(s, mse);

		IntVec3 ap   = subtract(p, a);
		IntVec3 c1   = cross(ab, ap);
		float check1 = dot(c1, n);

		IntVec3 bp = subtract(p, b);
		IntVec3 c2 = cross(bc, bp);
		float check2 = dot(c2, n);

		IntVec3 cp = subtract(p, c);
		IntVec3 c3 = cross(ca, cp);
		float check3 = dot(c3, n);

		if ((check1 >= 0.0 && check2 >= 0.0 && check3 >= 0.0)
	 	 || (check1 <= 0.0 && check2 <= 0.0 && check3 <= 0.0)) {
			intersection.mX = static_cast<uint32_t>(p.mX);
			intersection.mY = static_cast<uint32_t>(p.mY);
			intersection.mZ = static_cast<uint32_t>(p.mZ);
			return true;
		}
	}
	return false;
}

//////////////////////////////////////
void GeometricalUtility::adjustPosition(const IWorldNavigator::Vec3& start
	                                  , const IWorldNavigator::Vec3& intersection
	                                  , float distance
	                                  , IWorldNavigator::Vec3& adjusted)
{
	IntVec3 s = conv(start);
	IntVec3 i = conv(intersection);

	IntVec3 si = subtract(i, s);

	float t = 1 - (distance / sqrtf(si.mX * si.mX + si.mY * si.mY + si.mZ * si.mZ));

	IntVec3 tsi = multiply(t, si);

	IntVec3 result = add(s, tsi);

	adjusted.mX = static_cast<uint32_t>(result.mX);
	adjusted.mY = static_cast<uint32_t>(result.mY);
	adjusted.mZ = static_cast<uint32_t>(result.mZ);
}


