#ifndef ELLIPSOID_H
#define ELLIPSOID_H

#include <array>

namespace Ellipsoid
{
	float Q = 7.81f;

	struct EllipsoidGeneral
	{
		glm::vec4 mu;
		glm::vec4 sigma;
		glm::mat4 rotation;
		glm::mat4 covariance_invariant;
	};

	struct EllipsoidAddtitional
	{
		float sh_main[3];
		float sh_add[45];
		float opacity;
	};
};

#endif // !ELLIPSOID_H
