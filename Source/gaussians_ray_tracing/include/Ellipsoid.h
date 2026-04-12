#ifndef ELLIPSOID_H
#define ELLIPSOID_H

#include <glm/glm.hpp>

namespace Ellipsoid
{
	float Q = 1.0f;

	struct EllipsoidGeneral {
		glm::vec3 mu;                  // 12 байт
		float padding_mu;              // 4 байта (для выравнивания до 16)
		glm::vec3 sigma;               // 12 байт
		float padding_sigma;           // 4 байта
		glm::mat3 rotation;            // 36 байт
		glm::mat3 inverse_covariance;  // 36 байт
	};

	struct EllipsoidAdditional {
		float sh_main[3];
		float sh_rest[45];
		float opacity;
	};
}

#endif // !ELLIPSOID_H
