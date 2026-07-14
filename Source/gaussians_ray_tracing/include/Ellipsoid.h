#ifndef ELLIPSOID_H
#define ELLIPSOID_H

#include <array>

namespace Ellipsoid
{
	float Q = std::sqrt(7.81f);

	struct EllipsoidCPU
	{
		glm::vec3 mu;
		glm::vec3 sigma;
		glm::mat4 rotation;
	};

	struct EllipsoidGPU
	{
		glm::vec4 mu;
		glm::mat4 L;
	};

	struct EllipsoidAdditional
	{
		float sh_main[3];
		float sh_add[45];
		float opacity;
	};
};

#endif // !ELLIPSOID_H
