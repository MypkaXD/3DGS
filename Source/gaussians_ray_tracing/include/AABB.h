#ifndef AABB_H
#define AABB_H

#include <glm/glm.hpp>
#include <utility>
#include <cmath>

#include <Ellipsoid.h>

struct AABB
{
    glm::vec3 center;
    glm::vec3 extent;

    AABB(glm::vec3 center, glm::vec3 extent)
		: center(center), extent(extent) {
	}

    AABB(const Ellipsoid::EllipsoidGeneral& ellipsoid, const float Q)
    {
        auto x = calculate_global_coord(ellipsoid, Q, 0);
        auto y = calculate_global_coord(ellipsoid, Q, 1);
        auto z = calculate_global_coord(ellipsoid, Q, 2);

        float x_max = x.first.x;
        float y_max = y.first.y;
        float z_max = z.first.z;

        float x_min = x.second.x;
        float y_min = y.second.y;
        float z_min = z.second.z;

        glm::vec3 min(x_min, y_min, z_min);
        glm::vec3 max(x_max, y_max, z_max);

        center = (min + max) * 0.5f;
        extent = (max - min) * 0.5f;
    }

    static float cube_vertices[24];
    static unsigned int cube_indices[24];

private:

    glm::vec3 calculate_local_coord(const Ellipsoid::EllipsoidGeneral& ellipsoid, const float Q, const std::size_t idx)
    {
        float denominator =
            std::sqrt(
                ellipsoid.rotation[idx][0] * ellipsoid.rotation[idx][0] * ellipsoid.sigma[0] * ellipsoid.sigma[0] +
                ellipsoid.rotation[idx][1] * ellipsoid.rotation[idx][1] * ellipsoid.sigma[1] * ellipsoid.sigma[1] +
                ellipsoid.rotation[idx][2] * ellipsoid.rotation[idx][2] * ellipsoid.sigma[2] * ellipsoid.sigma[2]
            );

        float local_x = (ellipsoid.rotation[idx][0] * ellipsoid.sigma[0] * ellipsoid.sigma[0] * std::sqrt(Q)) / denominator;
        float local_y = (ellipsoid.rotation[idx][1] * ellipsoid.sigma[1] * ellipsoid.sigma[1] * std::sqrt(Q)) / denominator;
        float local_z = (ellipsoid.rotation[idx][2] * ellipsoid.sigma[2] * ellipsoid.sigma[2] * std::sqrt(Q)) / denominator;

        return glm::vec3(local_x, local_y, local_z);
    }

    std::pair<glm::vec3, glm::vec3> calculate_global_coord(const Ellipsoid::EllipsoidGeneral& ellipsoid, const float Q, const std::size_t idx)
    {
        glm::vec3 local = calculate_local_coord(ellipsoid, Q, idx);

		glm::vec3 mu = glm::vec3(ellipsoid.mu[0], ellipsoid.mu[1], ellipsoid.mu[2]);

        return {
            glm::mat3(ellipsoid.rotation) * local + mu,
            glm::mat3(ellipsoid.rotation) * (-local) + mu
        };
    }
};

float AABB::cube_vertices[24] = {
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f
};

unsigned int AABB::cube_indices[24] = {
    0,1, 1,2, 2,3, 3,0,
    4,5, 5,6, 6,7, 7,4,
    0,4, 1,5, 2,6, 3,7
};

void generate_AABB_from_gaussians(const std::vector<Ellipsoid::EllipsoidGeneral>& gaussians, std::vector<AABB>& aabb)
{
    aabb.clear();
    aabb.reserve(gaussians.size());

    for (std::size_t idx = 0; idx < gaussians.size(); ++idx)
    {
        aabb.emplace_back(gaussians[idx], Ellipsoid::Q);
    }
}

#endif