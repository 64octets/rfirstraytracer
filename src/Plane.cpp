#include "Plane.h"

Plane::Plane(plane_t plane) {
	mPlane = plane;

	type = PLANE;
};

bool Plane::intersect(const ray_t &ray, Intersection_t &hit) {
	float t = ray.intersectPlane(mPlane);
	if ( t != FLT_MAX ) {
		hit.t = t;
		hit.normal = mPlane.normal;
		return true;
	}
	return false;
};