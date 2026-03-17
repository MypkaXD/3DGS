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

#endif