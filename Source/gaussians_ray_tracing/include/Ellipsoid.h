#ifndef ELLIPSOID_H
#define ELLIPSOID_H

#include <glm/glm.hpp>

namespace Ellipsoid
{
	float Q = 1.0f;

	struct EllipsoidGeneral
	{
		glm::vec3 mu;
		glm::vec3 sigma;
		glm::mat3 rotation;
		glm::mat3 inverse_covariance;
	};

	struct EllipsoidAdditional
	{
		float sh_main[3];
		float sh_rest[45];
		float opacity;
	};
}

#endif // !ELLIPSOID_H
