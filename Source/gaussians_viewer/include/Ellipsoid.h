#ifndef ELLIPSOID_H
#define ELLIPSOID_H

#define _USE_MATH_DEFINES
#include <cmath>

#include <vector>

class Ellipsoid
{
private:

	double m_a;
	double m_b;
	double m_c;

	double m_q;

	std::vector<double> m_x;
	std::vector<double> m_y;
	std::vector<double> m_z;

	std::vector<unsigned int> m_indices;

public:

	Ellipsoid(double a, double b, double c, double q):
		m_a(a), m_b(b), m_c(c), m_q(q)
	{
	}

	double getA() const { return m_a; }
	double getB() const { return m_b; }
	double getC() const { return m_c; }
	double getQ() const { return m_q; }

	void generate(std::size_t n)
	{
		generate_vertices(n);
		generate_indices(n);
	}

private:
	void generate_vertices(std::size_t n)
	{

		m_x.clear();
		m_y.clear();
		m_z.clear();

		m_x.resize(n * n);
		m_y.resize(n * n);
		m_z.resize(n * n);

		double min_phi = 0.0;
		double max_phi = 2.0 * M_PI;

		double min_theta = 0.0;
		double max_theta = M_PI;

		double phi_step = (max_phi - min_phi) / (static_cast<double>(n) - 1);
		double theta_step = (max_theta - min_theta) / (static_cast<double>(n) - 1);

		double sqrt_q = std::sqrt(m_q);

		for (std::size_t idx_i = 0; idx_i < n; ++idx_i)
		{
			double phi = min_phi + static_cast<double>(idx_i) * phi_step;
			for (std::size_t idx_j = 0; idx_j < n; ++idx_j)
			{
				double theta = min_theta + static_cast<double>(idx_j) * theta_step;

				m_x[idx_i * n + idx_j] = m_a * sqrt_q * std::sin(theta) * std::cos(phi);
				m_y[idx_i * n + idx_j] = m_b * sqrt_q * std::sin(theta) * std::sin(phi);
				m_z[idx_i * n + idx_j] = m_c * sqrt_q * std::cos(theta);
			}
		}
	}

	void generate_indices(std::size_t n)
	{
		m_indices.clear();

		m_indices.resize((n - 1) * (n - 1) * 6);

		for (std::size_t idx_i = 0; idx_i < n - 1; ++idx_i)
		{
			for (std::size_t idx_j = 0; idx_j < n - 1; ++idx_j)
			{
				std::size_t idx_0 = idx_i * n + idx_j;
				std::size_t idx_1 = (idx_i + 1) * n + idx_j;
				std::size_t idx_2 = (idx_i + 1) * n + (idx_j + 1);
				std::size_t idx_3 = idx_i * n + (idx_j + 1);

				m_indices.push_back(idx_0);
				m_indices.push_back(idx_1);
				m_indices.push_back(idx_2);
				m_indices.push_back(idx_0);
				m_indices.push_back(idx_2);
				m_indices.push_back(idx_3);
			}
		}

	}
};

#endif // !ELLIPSOID_H
