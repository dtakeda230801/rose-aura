#pragma once

#include <vector>
#include "IWorldNavigator.h"

class GeometricalUtility {
public:

	using Vertex  = std::vector<IWorldNavigator::Vec3>;
	using Indices = std::vector<uint32_t>;

	static bool  equalVec3(IWorldNavigator::Vec3& a, IWorldNavigator::Vec3& b);

	static float calcAngle(float x, float y);

	static float calcDistance(IWorldNavigator::Vec3& a, IWorldNavigator::Vec3& b);
	
	static bool detectCollision(const Vertex& vertex
		                      , const Indices& indeices
		                      , const IWorldNavigator::Vec3& start
		                      , const IWorldNavigator::Vec3& end
	                          , IWorldNavigator::Vec3& intersection);

	static void adjustPosition(const IWorldNavigator::Vec3& start
		                     , const IWorldNavigator::Vec3& intersection
		                     , float distance
		                     , IWorldNavigator::Vec3& adjusted);

private:
	GeometricalUtility() = default;
	virtual ~GeometricalUtility() = default;
};
