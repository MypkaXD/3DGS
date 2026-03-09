#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>

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

		std::vector<double> a_values = { 0.0, 0.0};
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

		std::vector<double> a_values = { 0.0, 0.0, 0.0 };
		std::vector<double> sigma_values = { 0.5, 0.7, 0.9 };

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


int main()
{

	// create_2D();
	// create_3D();
	create_4D();


	return 0;
}