#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>
#include <array>
#include <random>

#define _USE_MATH_DEFINES

#include <math.h>


void dump3(std::ofstream& file, double x, double y, double z)
{
	file << "(" << x << "," << y << "," << z << ")";
}


void create_2D() {

	double a = 0.0; // mean of the Gaussian distribution (like point on x axis where the peak is located) (-inf, inf)
	double sigma = 0.05; // standard deviation of the Gaussian distribution (controls the width of the bell curve) (0.0, inf) if sigma is small, the bell curve will be narrow and tall; if sigma is large, the bell curve will be wide and short

	// use 3-sigma rule to determine the range of x values
	double x_min = a - 3.0 * sigma;
	double x_max = a + 3.0 * sigma;

	std::size_t n = 100000;

	double step = (x_max - x_min) / double(n - 1);

	std::ofstream file("gaussians_2D.txt");

	if (file.is_open())
	{
		std::vector<std::tuple<double, double, double>> points(n);

		for (std::size_t idx = 0; idx < n; ++idx)
		{
			double x = x_min + step * idx;
			double y = (1.0 / (sigma * std::sqrt(2.0 * M_PI))) * std::exp(-0.5 * std::pow((x - a) / sigma, 2));
			double z = 0.0;

			points[idx] = std::make_tuple(x, y, z);
		}

		file << "lines: gaussians_2D" << std::endl;

		for (std::size_t idx = 0; idx < n - 1; ++idx)
		{
			auto cur_point = &points[idx];
			auto next_point = &points[idx + 1];

			dump3(file, std::get<0>(*cur_point), std::get<1>(*cur_point), std::get<2>(*cur_point));
			dump3(file, std::get<0>(*next_point), std::get<1>(*next_point), std::get<2>(*next_point));
			dump3(file, 1.0, 0.0, 0.0);
			file << std::endl;
		}


		file.close();
	}
	else
	{
		return;
	}
}

void create_3D() {

	std::ofstream file("gaussians_3D.txt");

	if (file.is_open())
	{
		std::size_t n = 100;
		std::vector<std::vector<std::tuple<double, double, double>>> points(n, std::vector<std::tuple<double, double, double>>(n));

		std::vector<double> a_values = { 0.0, 0.0 };
		std::vector<double> sigma_values = { 0.1, 0.5 };

		double x_min = a_values[0] - 3.0 * sigma_values[0];
		double x_max = a_values[0] + 3.0 * sigma_values[0];

		double y_min = a_values[1] - 3.0 * sigma_values[1];
		double y_max = a_values[1] + 3.0 * sigma_values[1];

		double step_x = (x_max - x_min) / double(n - 1);
		double step_y = (y_max - y_min) / double(n - 1);

		for (std::size_t idx_i = 0; idx_i < n; ++idx_i)
		{
			double x = x_min + step_x * idx_i;
			for (std::size_t idx_j = 0; idx_j < n; ++idx_j)
			{
				double y = y_min + step_y * idx_j;

				double sigma_mult = sigma_values[0] * sigma_values[1];
				double exp = -0.5 * (std::pow(x - a_values[0], 2) / (sigma_values[0] * sigma_values[0]) + std::pow(y - a_values[1], 2) / (sigma_values[1] * sigma_values[1]));

				double z = 1.0 / (2.0 * M_PI * sigma_mult) * std::exp(exp);
				points[idx_i][idx_j] = std::make_tuple(x, y, z);
			}
		}

		file << "triangles: gaussians_3D" << std::endl;

		for (std::size_t idx_i = 0; idx_i < n - 1; ++idx_i)
		{
			for (std::size_t idx_j = 0; idx_j < n - 1; ++idx_j)
			{
				auto p1 = &points[idx_i][idx_j];
				auto p2 = &points[idx_i + 1][idx_j];
				auto p3 = &points[idx_i][idx_j + 1];
				auto p4 = &points[idx_i + 1][idx_j + 1];

				dump3(file, std::get<0>(*p1), std::get<1>(*p1), std::get<2>(*p1));
				dump3(file, std::get<0>(*p2), std::get<1>(*p2), std::get<2>(*p2));
				dump3(file, std::get<0>(*p3), std::get<1>(*p3), std::get<2>(*p3));
				dump3(file, 1.0, 0.0, 0.0);
				file << std::endl;

				dump3(file, std::get<0>(*p2), std::get<1>(*p2), std::get<2>(*p2));
				dump3(file, std::get<0>(*p3), std::get<1>(*p3), std::get<2>(*p3));
				dump3(file, std::get<0>(*p4), std::get<1>(*p4), std::get<2>(*p4));
				dump3(file, 1.0, 0.0, 0.0);
				file << std::endl;
			}
		}

		file.close();
	}
	else
	{
		return;
	}

}

void create_4D() {

	std::ofstream file("gaussians_4D.txt");

	if (file.is_open())
	{
		std::size_t n = 50;
		std::vector<std::tuple<double, double, double, double>> points(n * n * n);

		std::vector<double> a_values = { 10.0, 0.0, 0.0 };
		std::vector<double> sigma_values = { 10.0, 2.0, 3.0 };

		double x_min = a_values[0] - 3.0 * sigma_values[0];
		double x_max = a_values[0] + 3.0 * sigma_values[0];

		double y_min = a_values[1] - 3.0 * sigma_values[1];
		double y_max = a_values[1] + 3.0 * sigma_values[1];

		double z_min = a_values[2] - 3.0 * sigma_values[2];
		double z_max = a_values[2] + 3.0 * sigma_values[2];

		double step_x = (x_max - x_min) / double(n - 1);
		double step_y = (y_max - y_min) / double(n - 1);
		double step_z = (z_max - z_min) / double(n - 1);

		double min_v = std::numeric_limits<double>::max();
		double max_v = std::numeric_limits<double>::lowest();

		for (std::size_t idx_i = 0; idx_i < n; ++idx_i)
		{
			double x = x_min + step_x * idx_i;
			for (std::size_t idx_j = 0; idx_j < n; ++idx_j)
			{
				double y = y_min + step_y * idx_j;
				for (std::size_t idx_k = 0; idx_k < n; ++idx_k)
				{
					double z = z_min + step_z * idx_k;

					double v = 1.0 / (std::pow(2.0 * M_PI, 3.0 / 2.0) * sigma_values[0] * sigma_values[1] * sigma_values[2]);
					v *= std::exp(-0.5 * (
						std::pow(x - a_values[0], 2) / std::pow(sigma_values[0], 2) +
						std::pow(y - a_values[1], 2) / std::pow(sigma_values[1], 2) +
						std::pow(z - a_values[2], 2) / std::pow(sigma_values[2], 2)
						));

					if (v > max_v)
						max_v = v;
					if (v < min_v)
						min_v = v;

					points[idx_i * n * n + idx_j * n + idx_k] = std::make_tuple(x, y, z, v);
				}
			}
		}

		file << "points: gaussians_4D" << std::endl;

		for (std::size_t idx_i = 0; idx_i < n; ++idx_i)
		{
			for (std::size_t idx_j = 0; idx_j < n; ++idx_j)
			{
				for (std::size_t idx_k = 0; idx_k < n; ++idx_k)
				{
					auto p = &points[idx_i * n * n + idx_j * n + idx_k];
					double v = 1.0 + (std::get<3>(*p) - min_v) / (max_v - min_v);

					double eps = 0.01;

					if (v < 1.2 - eps || v > 1.2 + eps)
						continue;

					dump3(file, std::get<0>(*p), std::get<1>(*p), std::get<2>(*p));
					file << v;
					dump3(file, 1.0, 0.0, 0.0);
					file << std::endl;
				}
			}
		}

		file.close();
	}
	else
	{
		return;
	}

}

void create_ellipsoid_without_rotate()
{

	double min_phi = 0.0;
	double max_phi = 2 * M_PI;

	double min_theta = 0.0;
	double max_theta = M_PI;

	std::size_t n = 100;

	double step_phi = (max_phi - min_phi) / double(n - 1);
	double step_theta = (max_theta - min_theta) / double(n - 1);

	// double Q = 7.814727903251179; // 95%
	double Q = 1; // 19.87%

	std::vector<double> mu = { 10.0, 0.0, 0.0 };
	std::vector<double> sigma = { 10.0, 2.0, 3.0 };

	std::ofstream file("ellipsoid.txt");

	if (file.is_open())
	{
		file << "triangles: ellipsoid" << std::endl;

		for (std::size_t idx_i = 0; idx_i < n - 1; ++idx_i)
		{
			double phi_1 = min_phi + step_phi * idx_i;
			double phi_2 = min_phi + step_phi * (idx_i + 1);

			for (std::size_t idx_j = 0; idx_j < n - 1; ++idx_j)
			{
				double theta_1 = min_theta + step_theta * idx_j;
				double theta_2 = min_theta + step_theta * (idx_j + 1);

				double x1 = sigma[0] * std::sqrt(Q) * std::sin(theta_1) * std::cos(phi_1) + mu[0];
				double y1 = sigma[1] * std::sqrt(Q) * std::sin(theta_1) * std::sin(phi_1) + mu[1];
				double z1 = sigma[2] * std::sqrt(Q) * std::cos(theta_1) + +mu[2];

				double x2 = sigma[0] * std::sqrt(Q) * std::sin(theta_1) * std::cos(phi_2) + mu[0];
				double y2 = sigma[1] * std::sqrt(Q) * std::sin(theta_1) * std::sin(phi_2) + mu[1];
				double z2 = sigma[2] * std::sqrt(Q) * std::cos(theta_1) + +mu[2];

				double x3 = sigma[0] * std::sqrt(Q) * std::sin(theta_2) * std::cos(phi_2) + mu[0];
				double y3 = sigma[1] * std::sqrt(Q) * std::sin(theta_2) * std::sin(phi_2) + mu[1];
				double z3 = sigma[2] * std::sqrt(Q) * std::cos(theta_2) + +mu[2];

				double x4 = sigma[0] * std::sqrt(Q) * std::sin(theta_2) * std::cos(phi_1) + mu[0];
				double y4 = sigma[1] * std::sqrt(Q) * std::sin(theta_2) * std::sin(phi_1) + mu[1];
				double z4 = sigma[2] * std::sqrt(Q) * std::cos(theta_2) + +mu[2];

				dump3(file, x1, y1, z1);
				dump3(file, x2, y2, z2);
				dump3(file, x3, y3, z3);
				dump3(file, 1.0, 0.0, 0.0);
				file << "\n";

				dump3(file, x1, y1, z1);
				dump3(file, x3, y3, z3);
				dump3(file, x4, y4, z4);
				dump3(file, 1.0, 0.0, 0.0);
				file << "\n";
			}
		}

		// for ellipsoid without rotate
		file << "points: point" << std::endl;

		double x_1 = mu[0] + sigma[0] * std::sqrt(Q);
		double x_2 = mu[0] - sigma[0] * std::sqrt(Q);

		double y_1 = mu[1] + sigma[1] * std::sqrt(Q);
		double y_2 = mu[1] - sigma[1] * std::sqrt(Q);

		double z_1 = mu[2] + sigma[2] * std::sqrt(Q);
		double z_2 = mu[2] - sigma[2] * std::sqrt(Q);

		dump3(file, x_1, y_1, z_1);
		dump3(file, 1.0, 1.0, 1.0);
		file << "\n";
		dump3(file, x_2, y_2, z_2);
		dump3(file, 1.0, 1.0, 1.0);
		file << "\n";

		file.close();
	}

}

std::pair<std::array<float, 3>, std::array<float, 3>> get_local_coords(
	std::array<std::array<float, 3>, 3> R,
	float Q,
	std::array<float, 3> sigma,
	std::size_t coord  // координата, для которой ищем экстремум (0=x,1=y,2=z)
) {
	// Вычисляем A_i = √(R[coord][0]² σ_x² + R[coord][1]² σ_y² + R[coord][2]² σ_z²)
	float denominator = 0.0f;
	for (int j = 0; j < 3; j++) {
		denominator += R[coord][j] * R[coord][j] * sigma[j] * sigma[j];
	}
	denominator = std::sqrt(denominator);

	float sqrtQ = std::sqrt(Q);
	float scale = sqrtQ / denominator;

	std::array<float, 3> local_max, local_min;

	// Для каждой компоненты u1, u2, u3 используем соответствующий sigma[j]
	for (int j = 0; j < 3; j++) {
		local_max[j] = +R[coord][j] * sigma[j] * sigma[j] * scale;
		local_min[j] = -R[coord][j] * sigma[j] * sigma[j] * scale;
	}

	return std::make_pair(local_max, local_min);
}

std::vector<std::pair<float, float>> get_global_coords(
	std::array<std::array<float, 3>, 3> R,
	std::array<float, 3> sigma,
	std::array<float, 3> mu,
	float Q
) {
	std::vector<std::pair<float, float>> result;
	result.reserve(3);

	for (std::size_t coord = 0; coord < 3; ++coord) {
		auto local = get_local_coords(R, Q, sigma, coord);

		auto& local_max = local.first;  // локальные координаты для максимума
		auto& local_min = local.second; // локальные координаты для минимума

		// Поворачиваем: x_world = m + R * y_local
		float world_max = mu[coord];
		float world_min = mu[coord];

		for (int j = 0; j < 3; j++) {
			world_max += R[coord][j] * local_max[j];
			world_min += R[coord][j] * local_min[j];
		}

		result.emplace_back(world_max, world_min);
	}

	return result;
}

std::array<std::array<float, 3>, 3> quaternion_to_rotate_matrix(float x, float y, float z, float w)
{
	float xx = x * x;
	float xy = x * y;
	float xz = x * z;
	float xw = x * w;

	float yy = y * y;
	float yz = y * z;
	float yw = y * w;

	float zz = z * z;
	float zw = z * w;

	return std::array<std::array<float, 3>, 3>{{
			std::array<float, 3>{1.0f - 2.0f * (yy + zz), 2.0f * (xy - zw), 2.0f * (xz + yw)},
				std::array<float, 3>{2.0f * (xy + zw), 1.0f - 2.0f * (xx + zz), 2.0f * (yz - xw)},
				std::array<float, 3>{2.0f * (xz - yw), 2.0f * (yz + xw), 1.0f - 2.0f * (xx + yy)}
		}};
}

void create_ellipsoid_with_rotate()
{
	double min_phi = 0.0;
	double max_phi = 2 * M_PI;
	double min_theta = 0.0;
	double max_theta = M_PI;

	std::size_t n = 100;
	double step_phi = (max_phi - min_phi) / (n - 1);
	double step_theta = (max_theta - min_theta) / (n - 1);

	double Q = 7.814727903251179; // 95%
	double sqrtQ = std::sqrt(Q);

	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_int_distribution<int> dis_mu(-1000, 1000);
	std::uniform_real_distribution<float> dis_sigma(1, 2);
	std::normal_distribution<float> dis_normal(0.0f, 1.0f);

	//float sigma_x = dis_sigma(gen);
	//float sigma_y = dis_sigma(gen);
	//float sigma_z = dis_sigma(gen);

	//float mu_x = dis_mu(gen);
	//float mu_y = dis_mu(gen);
	//float mu_z = dis_mu(gen);

	//float x = dis_normal(gen);
	//float y = dis_normal(gen);
	//float z = dis_normal(gen);
	//float w = dis_normal(gen);

	float sigma_x = 1.04682517f;
	float sigma_y = 1.67131209f;
	float sigma_z = 1.05468035f;

	float mu_x = 1.0f;
	float mu_y = 1.0f;
	float mu_z = -1.0f;

	float x = 0.586289942;
	float y = -0.767705917;
	float z = 0.112539463;
	float w = 0.232865825;

	float norm = std::sqrt(x * x + y * y + z * z + w * w);

	float qx = x / norm;
	float qy = y / norm;
	float qz = z / norm;
	float qw = w / norm;

	auto R = quaternion_to_rotate_matrix(qx, qy, qz, qw);

	std::ofstream file("ellipsoid.txt");

	if (file.is_open())
	{
		file << "triangles: ellipsoid" << std::endl;

		for (std::size_t i = 0; i < n - 1; ++i)
		{
			double phi1 = min_phi + step_phi * i;
			double phi2 = min_phi + step_phi * (i + 1);

			for (std::size_t j = 0; j < n - 1; ++j)
			{
				double theta1 = min_theta + step_theta * j;
				double theta2 = min_theta + step_theta * (j + 1);

				// (phi1, theta1)
				double sphere_x1 = std::sin(theta1) * std::cos(phi1);
				double sphere_y1 = std::sin(theta1) * std::sin(phi1);
				double sphere_z1 = std::cos(theta1);

				double scaled_x1 = sphere_x1 * sigma_x;
				double scaled_y1 = sphere_y1 * sigma_y;
				double scaled_z1 = sphere_z1 * sigma_z;

				double world_x1 = mu_x + sqrtQ * (R[0][0] * scaled_x1 + R[0][1] * scaled_y1 + R[0][2] * scaled_z1);
				double world_y1 = mu_y + sqrtQ * (R[1][0] * scaled_x1 + R[1][1] * scaled_y1 + R[1][2] * scaled_z1);
				double world_z1 = mu_z + sqrtQ * (R[2][0] * scaled_x1 + R[2][1] * scaled_y1 + R[2][2] * scaled_z1);

				// (phi2, theta1)
				double sphere_x2 = std::sin(theta1) * std::cos(phi2);
				double sphere_y2 = std::sin(theta1) * std::sin(phi2);
				double sphere_z2 = std::cos(theta1);

				double scaled_x2 = sphere_x2 * sigma_x;
				double scaled_y2 = sphere_y2 * sigma_y;
				double scaled_z2 = sphere_z2 * sigma_z;

				double world_x2 = mu_x + sqrtQ * (R[0][0] * scaled_x2 + R[0][1] * scaled_y2 + R[0][2] * scaled_z2);
				double world_y2 = mu_y + sqrtQ * (R[1][0] * scaled_x2 + R[1][1] * scaled_y2 + R[1][2] * scaled_z2);
				double world_z2 = mu_z + sqrtQ * (R[2][0] * scaled_x2 + R[2][1] * scaled_y2 + R[2][2] * scaled_z2);

				// (phi2, theta2)
				double sphere_x3 = std::sin(theta2) * std::cos(phi2);
				double sphere_y3 = std::sin(theta2) * std::sin(phi2);
				double sphere_z3 = std::cos(theta2);

				double scaled_x3 = sphere_x3 * sigma_x;
				double scaled_y3 = sphere_y3 * sigma_y;
				double scaled_z3 = sphere_z3 * sigma_z;

				double world_x3 = mu_x + sqrtQ * (R[0][0] * scaled_x3 + R[0][1] * scaled_y3 + R[0][2] * scaled_z3);
				double world_y3 = mu_y + sqrtQ * (R[1][0] * scaled_x3 + R[1][1] * scaled_y3 + R[1][2] * scaled_z3);
				double world_z3 = mu_z + sqrtQ * (R[2][0] * scaled_x3 + R[2][1] * scaled_y3 + R[2][2] * scaled_z3);

				// (phi1, theta2)
				double sphere_x4 = std::sin(theta2) * std::cos(phi1);
				double sphere_y4 = std::sin(theta2) * std::sin(phi1);
				double sphere_z4 = std::cos(theta2);

				double scaled_x4 = sphere_x4 * sigma_x;
				double scaled_y4 = sphere_y4 * sigma_y;
				double scaled_z4 = sphere_z4 * sigma_z;

				double world_x4 = mu_x + sqrtQ * (R[0][0] * scaled_x4 + R[0][1] * scaled_y4 + R[0][2] * scaled_z4);
				double world_y4 = mu_y + sqrtQ * (R[1][0] * scaled_x4 + R[1][1] * scaled_y4 + R[1][2] * scaled_z4);
				double world_z4 = mu_z + sqrtQ * (R[2][0] * scaled_x4 + R[2][1] * scaled_y4 + R[2][2] * scaled_z4);

				dump3(file, world_x1, world_y1, world_z1);
				dump3(file, world_x2, world_y2, world_z2);
				dump3(file, world_x3, world_y3, world_z3);
				dump3(file, 1.0, 0.0, 0.0);
				file << "\n";

				dump3(file, world_x1, world_y1, world_z1);
				dump3(file, world_x3, world_y3, world_z3);
				dump3(file, world_x4, world_y4, world_z4);
				dump3(file, 1.0, 0.0, 0.0);
				file << "\n";
			}
		}

		auto extrems = get_global_coords(R, { sigma_x, sigma_y, sigma_z }, { mu_x, mu_y, mu_z }, Q);

		file << "lines: AABB" << std::endl;

		double x_min = extrems[0].second;  // min x
		double x_max = extrems[0].first;   // max x
		double y_min = extrems[1].second;  // min y
		double y_max = extrems[1].first;   // max y
		double z_min = extrems[2].second;  // min z
		double z_max = extrems[2].first;   // max z

		// Определяем 8 вершин AABB
		// Задняя нижняя грань
		dump3(file, x_min, y_min, z_min); dump3(file, x_max, y_min, z_min); dump3(file, 1.0, 1.0, 1.0); file << "\n";
		dump3(file, x_max, y_min, z_min); dump3(file, x_max, y_max, z_min); dump3(file, 1.0, 1.0, 1.0); file << "\n";
		dump3(file, x_max, y_max, z_min); dump3(file, x_min, y_max, z_min); dump3(file, 1.0, 1.0, 1.0); file << "\n";
		dump3(file, x_min, y_max, z_min); dump3(file, x_min, y_min, z_min); dump3(file, 1.0, 1.0, 1.0); file << "\n";

		// Передняя грань (z_max)
		dump3(file, x_min, y_min, z_max); dump3(file, x_max, y_min, z_max); dump3(file, 1.0, 1.0, 1.0); file << "\n";
		dump3(file, x_max, y_min, z_max); dump3(file, x_max, y_max, z_max); dump3(file, 1.0, 1.0, 1.0); file << "\n";
		dump3(file, x_max, y_max, z_max); dump3(file, x_min, y_max, z_max); dump3(file, 1.0, 1.0, 1.0); file << "\n";
		dump3(file, x_min, y_max, z_max); dump3(file, x_min, y_min, z_max); dump3(file, 1.0, 1.0, 1.0); file << "\n";

		// Соединения между гранями (вертикальные ребра)
		dump3(file, x_min, y_min, z_min); dump3(file, x_min, y_min, z_max); dump3(file, 1.0, 1.0, 1.0); file << "\n";
		dump3(file, x_max, y_min, z_min); dump3(file, x_max, y_min, z_max); dump3(file, 1.0, 1.0, 1.0); file << "\n";
		dump3(file, x_max, y_max, z_min); dump3(file, x_max, y_max, z_max); dump3(file, 1.0, 1.0, 1.0); file << "\n";
		dump3(file, x_min, y_max, z_min); dump3(file, x_min, y_max, z_max); dump3(file, 1.0, 1.0, 1.0); file << "\n";


		file.close();
	}
}

int main()
{

	// create_2D();
	// create_3D();
	// create_4D();
	// create_ellipsoid_without_rotate();
	create_ellipsoid_with_rotate();

	return 0;
}