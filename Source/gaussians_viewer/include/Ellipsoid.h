#ifndef ELLIPSOID_H
#define ELLIPSOID_H

#define _USE_MATH_DEFINES
#include <math.h>

#include <vector>

class Ellipsoid
{
private:

	float m_a;
	float m_b;
	float m_c;

	float m_q;

	std::vector<float> m_vertices;
	std::vector<unsigned int> m_indices;

public:

	Ellipsoid(float a, float b, float c, float q):
		m_a(a), m_b(b), m_c(c), m_q(q)
	{
	}

	float getA() const { return m_a; }
	float getB() const { return m_b; }
	float getC() const { return m_c; }
	float getQ() const { return m_q; }

	void generate(std::size_t n)
	{
		generate_vertices(n);
		generate_indices(n);
	}

	std::size_t get_vertex_size()
	{
		return (m_vertices.size()) * sizeof(float);
	}

	std::size_t get_indices_size()
	{
		return m_indices.size() * sizeof(unsigned int);
	}

	std::vector<float>& get_vertices() { return m_vertices; }
	std::vector<unsigned int>& get_indices() { return m_indices; }

private:
	void generate_vertices(std::size_t n)
	{

		m_vertices.clear();
		
		m_vertices.resize(n * n * 3);

		float min_phi = 0.0;
		float max_phi = 2.0 * M_PI;

		float min_theta = 0.0;
		float max_theta = M_PI;

		float phi_step = (max_phi - min_phi) / (static_cast<float>(n) - 1);
		float theta_step = (max_theta - min_theta) / (static_cast<float>(n) - 1);

		float sqrt_q = std::sqrt(m_q);

		for (std::size_t idx_i = 0; idx_i < n; ++idx_i)
		{
			float phi = min_phi + static_cast<float>(idx_i) * phi_step;
			for (std::size_t idx_j = 0; idx_j < n; ++idx_j)
			{
				float theta = min_theta + static_cast<float>(idx_j) * theta_step;

				int vertex_index = (idx_i * n + idx_j) * 3;

				m_vertices[vertex_index + 0] = m_a * sqrt_q * std::sin(theta) * std::cos(phi);
				m_vertices[vertex_index + 1] = m_b * sqrt_q * std::sin(theta) * std::sin(phi);
				m_vertices[vertex_index + 2] = m_c * sqrt_q * std::cos(theta);
			}
		}
	}

	void generate_indices(std::size_t n)
	{
		m_indices.clear();

		m_indices.reserve((n - 1) * (n - 1) * 6);

		for (std::size_t idx_i = 0; idx_i < n - 1; ++idx_i)
		{
			for (std::size_t idx_j = 0; idx_j < n - 1; ++idx_j)
			{
				std::size_t idx_0 = idx_i * n + idx_j;
				std::size_t idx_1 = (idx_i + 1) * n + idx_j;
				std::size_t idx_2 = (idx_i + 1) * n + (idx_j + 1);
				std::size_t idx_3 = idx_i * n + (idx_j + 1);

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

#endif // !ELLIPSOID_H
