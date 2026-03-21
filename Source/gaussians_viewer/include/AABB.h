#ifndef AABB_H
#define AABB_H

#include <glm/glm.hpp>
#include <utility>
#include <cmath>

struct AABB
{
    glm::vec3 center;
    glm::vec3 extent;

    AABB(glm::vec3 center, glm::vec3 extent)
		: center(center), extent(extent) {
	}

    AABB(const glm::mat3& R,
        const glm::vec3& mu,
        const glm::vec3& sigma,
        float Q)
    {
        auto x = calculate_global_coord(R, mu, sigma, Q, 0);
        auto y = calculate_global_coord(R, mu, sigma, Q, 1);
        auto z = calculate_global_coord(R, mu, sigma, Q, 2);

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

    glm::vec3 calculate_local_coord(const glm::mat3& R,
        const glm::vec3& sigma,
        float Q,
        std::size_t idx)
    {
        float denominator =
            std::sqrt(
                R[0][idx] * R[0][idx] * sigma[0] * sigma[0] +
                R[1][idx] * R[1][idx] * sigma[1] * sigma[1] +
                R[2][idx] * R[2][idx] * sigma[2] * sigma[2]
            );

        float local_x = (R[0][idx] * sigma[0] * sigma[0] * std::sqrt(Q)) / denominator;
        float local_y = (R[1][idx] * sigma[1] * sigma[1] * std::sqrt(Q)) / denominator;
        float local_z = (R[2][idx] * sigma[2] * sigma[2] * std::sqrt(Q)) / denominator;

        return glm::vec3(local_x, local_y, local_z);
    }

    std::pair<glm::vec3, glm::vec3> calculate_global_coord(
        const glm::mat3& R,
        const glm::vec3& mu,
        const glm::vec3& sigma,
        float Q,
        std::size_t idx)
    {
        glm::vec3 local = calculate_local_coord(R, sigma, Q, idx);

        return {
            R * local + mu,
            R * (-local) + mu
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

void generate_AABB_from_gaussians(const std::vector<Ellipsoid>& gaussians, std::vector<AABB>& aabb)
{
    aabb.clear();
    aabb.reserve(gaussians.size());

    for (std::size_t idx = 0; idx < gaussians.size(); ++idx)
    {

        float x = gaussians[idx].quaternion[0];
        float y = gaussians[idx].quaternion[1];
        float z = gaussians[idx].quaternion[2];
        float w = gaussians[idx].quaternion[3];

        float xx = x * x;
        float xy = x * y;
        float xz = x * z;
        float xw = x * w;

        float yy = y * y;
        float yz = y * z;
        float yw = y * w;

        float zz = z * z;
        float zw = z * w;

        glm::mat3 R = glm::mat3{
            1.0 - 2.0 * (yy + zz), 2.0 * (xy - zw), 2.0 * (xz + yw),
            2.0 * (xy + zw), 1.0 - 2.0 * (xx + zz), 2.0 * (yz - xw),
            2.0 * (xz - yw), 2.0 * (yz + xw), 1.0 - 2.0 * (xx + yy)
        };

        glm::vec3 mu = glm::vec3{ gaussians[idx].mu[0], gaussians[idx].mu[1], gaussians[idx].mu[2] };
        glm::vec3 sigma = glm::vec3{ gaussians[idx].sigma[0], gaussians[idx].sigma[1], gaussians[idx].sigma[2] };

        aabb.emplace_back(R, mu, sigma, 7.91);
    }
}

#endif