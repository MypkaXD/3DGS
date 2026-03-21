#ifndef ELLIPSOID_H
#define ELLIPSOID_H

#include <array>

struct Ellipsoid
{
	using vec3 = std::array<float, 3>;
	using vec4 = std::array<float, 4>;

	vec3 mu;
	vec3 sigma;
	vec4 quaternion;

	float opacity;
	float Q;

	Ellipsoid(vec3 mu, vec3 sigma, vec4 quaternion, float Q, float opacity) : mu(mu), sigma(sigma), quaternion(quaternion), Q(Q), opacity(opacity)
	{
	}
};

#endif // !ELLIPSOID_H
