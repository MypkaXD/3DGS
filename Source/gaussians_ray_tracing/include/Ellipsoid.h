#ifndef ELLIPSOID_H
#define ELLIPSOID_H

#include <array>

namespace Ellipsoid
{
	float Q = 1.0f;

	struct EllipsoidGeneral
	{
		glm::vec4 mu;
		glm::vec4 sigma;
		glm::mat4 rotation;
		glm::mat4 covariance_invariant;
	};

	struct EllipsoidAddtitional
	{
		glm::vec4 sh_main;
		glm::vec4 sh_add[15];
		float opacity;
	};
};

#endif // !ELLIPSOID_H
