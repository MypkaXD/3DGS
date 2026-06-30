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
        auto r_1 = (glm::transpose(ellipsoid.rotation))[0];
        auto r_2 = (glm::transpose(ellipsoid.rotation))[1];
        auto r_3 = (glm::transpose(ellipsoid.rotation))[2];

        glm::vec3 min = glm::vec3(
            ellipsoid.mu[0] - float(Ellipsoid::Q * std::sqrt((glm::dot(r_1 * r_1, ellipsoid.sigma * ellipsoid.sigma)))),
            ellipsoid.mu[1] - float(Ellipsoid::Q * std::sqrt((glm::dot(r_2 * r_2, ellipsoid.sigma * ellipsoid.sigma)))),
            ellipsoid.mu[2] - float(Ellipsoid::Q * std::sqrt((glm::dot(r_3 * r_3, ellipsoid.sigma * ellipsoid.sigma))))
        );

        glm::vec3 max = glm::vec3(
            ellipsoid.mu[0] + float(Ellipsoid::Q * std::sqrt((glm::dot(r_1 * r_1, ellipsoid.sigma * ellipsoid.sigma)))),
            ellipsoid.mu[1] + float(Ellipsoid::Q * std::sqrt((glm::dot(r_2 * r_2, ellipsoid.sigma * ellipsoid.sigma)))),
            ellipsoid.mu[2] + float(Ellipsoid::Q * std::sqrt((glm::dot(r_3 * r_3, ellipsoid.sigma * ellipsoid.sigma))))
        );

        center = (min + max) * 0.5f;
        extent = (max - min) * 0.5f;
    }

    static float cube_vertices[24];
    static unsigned int cube_indices[24];
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