#ifndef ELLIPSOID_H
#define ELLIPSOID_H

#define _USE_MATH_DEFINES
#include <math.h>

#include <vector>
#include <array>

class UV
{
private:
	std::size_t m_count_u;
	std::size_t m_count_v;

	float min_u;
	float max_u;

	float min_v;
	float max_v;

	std::vector<float> m_vertices;
	std::vector<unsigned int> m_indices;

public:

	UV(std::size_t count_u, std::size_t count_v, float min_u, float max_u, float min_v, float max_v):
		m_count_u(count_u), m_count_v(count_v), min_u(min_u), max_u(max_u), min_v(min_v), max_v(max_v)
	{
	}

	void generate()
	{
		generate_vertices();
		generate_indices();
	}

	const std::vector<float>& get_vertices() const
	{
		return m_vertices;
	}

	const std::vector<unsigned int>& get_indices() const
	{
		return m_indices;
	}

private:
	void generate_vertices()
	{
		m_vertices.clear();
		
		m_vertices.resize(m_count_u * m_count_v * 2);
		float u_step = (max_u - min_u) / (static_cast<float>(m_count_u) - 1);
		float v_step = (max_v - min_v) / (static_cast<float>(m_count_v) - 1);
		for (std::size_t idx_i = 0; idx_i < m_count_u; ++idx_i)
		{
			float u = min_u + static_cast<float>(idx_i) * u_step;
			for (std::size_t idx_j = 0; idx_j < m_count_v; ++idx_j)
			{
				float v = min_v + static_cast<float>(idx_j) * v_step;
				int vertex_index = (idx_i * m_count_v + idx_j) * 2;
				m_vertices[vertex_index + 0] = u;
				m_vertices[vertex_index + 1] = v;
			}
		}
	}

	void generate_indices()
	{
		m_indices.clear();
		m_indices.reserve((m_count_u - 1) * (m_count_v - 1) * 6);
		for (std::size_t idx_i = 0; idx_i < m_count_u - 1; ++idx_i)
		{
			for (std::size_t idx_j = 0; idx_j < m_count_v - 1; ++idx_j)
			{
				std::size_t idx_0 = idx_i * m_count_v + idx_j;
				std::size_t idx_1 = (idx_i + 1) * m_count_v + idx_j;
				std::size_t idx_2 = (idx_i + 1) * m_count_v + (idx_j + 1);
				std::size_t idx_3 = idx_i * m_count_v + (idx_j + 1);
				m_indices.emplace_back(idx_0);
				m_indices.emplace_back(idx_1);
				m_indices.emplace_back(idx_2);
				m_indices.emplace_back(idx_0);
				m_indices.emplace_back(idx_2);
				m_indices.emplace_back(idx_3);
			}
		}
	}
};

struct Ellipsoid
{
	std::array<float, 3> mu;
	std::array<float, 3> sigma;
	std::array<float, 4> quaternion;
	float q;

	Ellipsoid(std::array<float, 3> mu, std::array<float, 3> sigma, std::array<float, 4> quaternion, float q) : mu(mu), sigma(sigma), quaternion(quaternion), q(q)
	{
	}
};

#endif // !ELLIPSOID_H
