#ifndef ELLIPSOID_H
#define ELLIPSOID_H

#include <array>

struct Ellipsoid
{
	float mu[3];
	float sigma[3];
	float quaternion[4];
	float sh_dc[3];
	float sh_rest[45];

	float opacity;
	float Q;

	Ellipsoid() {}
};

#endif // !ELLIPSOID_H
