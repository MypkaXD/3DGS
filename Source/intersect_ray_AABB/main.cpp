#include <iostream>
#include <array>
#include <fstream>

void dump3(std::ofstream& file, float x, float y, float z)
{
	file << "(" << x << "," << y << "," << z << ")";
}

int main()
{

	std::array<float, 3> min = { -2.0f, -5.0f, -25.0f };
	std::array<float, 3> max = { 5.0f, 5.0f, 5.0f };

	std::array<float, 3> center = { (min[0] + max[0]) / 2.0f, (min[1] + max[1]) / 2.0f, (min[2] + max[2]) / 2.0f };

	std::array<float, 3> camera_pos = { 0.0f, 0.0f, 10.0f };
	std::array<float, 3> camera_target = { 0.0f, 0.0f, 0.0f };

	std::ofstream file("output.txt");

	if (file.is_open())
	{
		double x_min = min[0];  // min x
		double x_max = max[0];  // max x
		double y_min = min[1];  // min y
		double y_max = max[1];  // max y
		double z_min = min[2];  // min z
		double z_max = max[2];  // max z

		file << "lines: AABB" << std::endl;

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

		file << "points: center" << std::endl;
		dump3(file, center[0], center[1], center[2]);
		file << "5"; 
		dump3(file, 1.0, 0.0, 0.0); file << "\n";

		file << "points: min" << std::endl;
		dump3(file, min[0], min[1], min[2]); 
		file << "5"; 
		dump3(file, 0.0, 1.0, 0.0); file << "\n";

		file << "points: max" << std::endl;
		dump3(file, max[0], max[1], max[2]);
		file << "5";
		dump3(file, 0.0, 0.0, 1.0); file << "\n";

		file << "points: camera_pos" << std::endl;
		dump3(file, camera_pos[0], camera_pos[1], camera_pos[2]);
		file << "2";
		dump3(file, 1.0, 1.0, 0.0); file << "\n";

		file << "vectors: camera" << std::endl;
		dump3(file, camera_pos[0] - camera_target[0], camera_pos[1] - camera_target[1], camera_pos[2] - camera_target[2]);
		dump3(file, 0.0, 1.0, 0.0);
		file << "\n";

		std::array<float, 3> t_min = { (camera_pos[0] - x_min) / (camera_pos[0] - camera_target[0]), (camera_pos[1] - y_min) / (camera_pos[1] - camera_target[1]), (camera_pos[2] - z_min) / (camera_pos[2] - camera_target[2]) };
		std::array<float, 3> t_max = { (camera_pos[0] - x_max) / (camera_pos[0] - camera_target[0]), (camera_pos[1] - y_max) / (camera_pos[1] - camera_target[1]), (camera_pos[2] - z_max) / (camera_pos[2] - camera_target[2]) };

		std::array<float, 3> t_enter = { std::min(t_min[0], t_max[0]), std::min(t_min[1], t_max[1]), std::min(t_min[2], t_max[2]) };
		std::array<float, 3> t_exit = { std::max(t_min[0], t_max[0]), std::max(t_min[1], t_max[1]), std::max(t_min[2], t_max[2]) };

		float t_enter_max = std::max({ t_enter[0], t_enter[1], t_enter[2] });
		float t_exit_min = std::min({ t_exit[0], t_exit[1], t_exit[2] });

		if (t_enter_max < t_exit_min && t_exit_min > 0)
		{
			std::array<float, 3> intersection_point = { camera_pos[0] + t_enter_max * (camera_target[0] - camera_pos[0]), camera_pos[1] + t_enter_max * (camera_target[1] - camera_pos[1]), camera_pos[2] + t_enter_max * (camera_target[2] - camera_pos[2]) };
			file << "points: intersection" << std::endl;
			dump3(file, intersection_point[0], intersection_point[1], intersection_point[2]);
			file << "5";
			dump3(file, 1.0, 0.0, 1.0); file << "\n";
		}
		else
		{
			std::cerr << "No intersection with AABB.\n";
		}

		file.close();
	}
	else
	{
		std::cerr << "Unable to open file for writing.\n";
	}


	return 0;
}