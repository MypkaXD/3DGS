#ifndef SCENE_LOADER_H
#define SCENE_LOADER_H

#include <string>
#include <iostream>
#include <fstream>
#include <tinyply.h>
#include <random>
#include <vector>

#include <Ellipsoid.h>

class SceneLoader
{
private:
	std::vector<Ellipsoid::EllipsoidGeneral> gaussians_general;
	std::vector<Ellipsoid::EllipsoidAddtitional> gaussians_addiotional;
public:

	SceneLoader() {}

	void create_random_scene(const std::size_t n)
	{
		std::random_device rd;
		std::mt19937 gen(rd());

		std::uniform_int_distribution<int> dis_mu(-5, 5);
		std::uniform_real_distribution<float> dis_sigma(1, 5);
		std::normal_distribution<float> dis_normal(0.0f, 1.0f);
		std::uniform_real_distribution<float> dis_uniform_0_to_1(0.0f, 1.0f);

		gaussians_general.clear();
		gaussians_addiotional.clear();

		gaussians_general.reserve(n);
		gaussians_addiotional.reserve(n);

		for (std::size_t idx = 0; idx < n; ++idx)
		{
			Ellipsoid::EllipsoidGeneral e_gen;
			Ellipsoid::EllipsoidAddtitional e_add;

			e_gen.mu[0] = dis_mu(gen);
			e_gen.mu[1] = dis_mu(gen);
			e_gen.mu[2] = dis_mu(gen);

			e_gen.sigma[0] = dis_sigma(gen);
			e_gen.sigma[1] = dis_sigma(gen);
			e_gen.sigma[2] = dis_sigma(gen);

			float x = dis_normal(gen);
			float y = dis_normal(gen);
			float z = dis_normal(gen);
			float w = dis_normal(gen);

			float norm = std::sqrt(x * x + y * y + z * z + w * w);

			e_gen.rotation = get_rotation_matrix_from_quaternion(glm::vec4(w / norm, x / norm, y / norm, z / norm));

			glm::mat4 scale_invatiant_squared = glm::mat4(
				1.0 / (e_gen.sigma[0] * e_gen.sigma[0]), 0.0f, 0.0f, 0.0f,
				0.0f, 1.0 / (e_gen.sigma[1] * e_gen.sigma[1]), 0.0f, 0.0f,
				0.0f, 0.0f, 1.0 / (e_gen.sigma[2] * e_gen.sigma[2]), 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			);

			// e_gen.covariance_invariant = e_gen.rotation * scale_invatiant_squared * glm::transpose(e_gen.rotation);
			e_gen.covariance_invariant =glm::mat4(
				1.0 / (e_gen.sigma[0]), 0.0f, 0.0f, 0.0f,
				0.0f, 1.0 / (e_gen.sigma[1]), 0.0f, 0.0f,
				0.0f, 0.0f, 1.0 / (e_gen.sigma[2]), 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			) * glm::transpose(e_gen.rotation);

			// std::cout << e_gen.covariance_invariant << std::endl;

			e_add.sh_main[0] = dis_uniform_0_to_1(gen);
			e_add.sh_main[1] = dis_uniform_0_to_1(gen);
			e_add.sh_main[2] = dis_uniform_0_to_1(gen);

			for (std::size_t idx_sh = 0; idx_sh < 45; ++idx_sh)
			{
				e_add.sh_add[idx_sh] = dis_uniform_0_to_1(gen);
			}

			e_add.opacity = dis_uniform_0_to_1(gen);
			// e_add.opacity = 0.2;

			gaussians_general.emplace_back(e_gen);
			gaussians_addiotional.emplace_back(e_add);
		}
	}

	void create_scene_from_file(const std::string& path_to_ply)
	{
		std::ifstream file_stream(path_to_ply, std::ios::binary);

		if (!file_stream)
			throw std::runtime_error("file_stream failed to open " + path_to_ply);

		file_stream.seekg(0, std::ios::end);
		const float size_mb = file_stream.tellg() * float(1e-6);
		file_stream.seekg(0, std::ios::beg);

		tinyply::PlyFile splat_ply;
		splat_ply.parse_header(file_stream);

		std::cout << "\t[ply_header] Type: " << (splat_ply.is_binary_file() ? "binary" : "ascii") << std::endl;
		for (const auto& c : splat_ply.get_comments())
			std::cout << "\t[ply_header] Comment: " << c << std::endl;
		for (const auto& c : splat_ply.get_info())
			std::cout << "\t[ply_header] Info: " << c << std::endl;

		for (const auto& e : splat_ply.get_elements())
		{
			std::cout << "\t[ply_header] element: " << e.name << " (" << e.size << ")" << std::endl;
			for (const auto& p : e.properties)
			{
				std::cout << "\t[ply_header] \tproperty: " << p.name << " (type=" << tinyply::PropertyTable[p.propertyType].str << ")";
				if (p.isList) std::cout << " (list_type=" << tinyply::PropertyTable[p.listType].str << ")";
				std::cout << std::endl;
			}
		}

		std::shared_ptr<tinyply::PlyData> xyz, nrm, f_dc, f_rest, opacity, scales, quat_rot;

		try { xyz = splat_ply.request_properties_from_element("vertex", { "x", "y", "z" }); }
		catch (const std::exception& e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		try { nrm = splat_ply.request_properties_from_element("vertex", { "nx", "ny", "nz" }); }
		catch (const std::exception& e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		try { opacity = splat_ply.request_properties_from_element("vertex", { "opacity" }); }
		catch (const std::exception& e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		try { scales = splat_ply.request_properties_from_element("vertex", { "scale_0", "scale_1", "scale_2" }); }
		catch (const std::exception& e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		try { quat_rot = splat_ply.request_properties_from_element("vertex", { "rot_0", "rot_1", "rot_2", "rot_3" }); }
		catch (const std::exception& e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		std::vector<std::string> sh_dc_names;
		std::vector<std::string> sh_rest_names;

		std::vector<tinyply::PlyElement> elements = splat_ply.get_elements();
		auto vertex_element_iter = std::find_if(elements.begin(), elements.end(), [](const tinyply::PlyElement& e) { return e.name == "vertex"; });

		if (vertex_element_iter == elements.end())
		{
			throw std::runtime_error("[error] no vertex element found in ply file");
		}

		for (const auto& prop : vertex_element_iter->properties)
		{
			if (prop.name.substr(0, 5) == "f_dc_") { sh_dc_names.push_back(prop.name); }
			else if (prop.name.substr(0, 7) == "f_rest_") { sh_rest_names.push_back(prop.name); }
		}

		// spherical harmonic coefficients
		try { f_dc = splat_ply.request_properties_from_element("vertex", sh_dc_names); }
		catch (const std::exception& e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

		try
		{
			f_rest = splat_ply.request_properties_from_element("vertex", sh_rest_names);
		}
		catch (...)
		{
			f_rest.reset(); // not an error if there are no f_rest components
		}

		splat_ply.read(file_stream);

		if (xyz)      std::cout << "\tRead " << xyz->count << " total xyz " << std::endl;
		if (nrm)      std::cout << "\tRead " << nrm->count << " total normals " << std::endl;
		if (opacity)  std::cout << "\tRead " << opacity->count << " total opacity " << std::endl;
		if (scales)   std::cout << "\tRead " << scales->count << " total scales " << std::endl;
		if (quat_rot) std::cout << "\tRead " << quat_rot->count << " total quaternion rotations" << std::endl;
		if (f_dc)     std::cout << "\tRead " << f_dc->count << " total sh f_dc " << std::endl;
		if (f_rest)   std::cout << "\tRead " << f_rest->count << " total sh f_rest " << std::endl;

		std::ofstream dump_file("dump.txt");

		for (std::size_t idx = 0; idx < xyz->count; ++idx)
		{
			dump_file << idx << "\n";

			Ellipsoid::EllipsoidGeneral e_gen;
			Ellipsoid::EllipsoidAddtitional e_add;

			e_gen.mu[0] = reinterpret_cast<float*>(xyz->buffer.get())[idx * 3 + 0];
			e_gen.mu[1] = reinterpret_cast<float*>(xyz->buffer.get())[idx * 3 + 1];
			e_gen.mu[2] = reinterpret_cast<float*>(xyz->buffer.get())[idx * 3 + 2];
			e_gen.mu[3] = 1.0f;

			//dump_file << "pos\n";
			//dump_file << "pos_x=" << e_gen.mu[0] << "\n";
			//dump_file << "pos_y=" << e_gen.mu[1] << "\n";
			//dump_file << "pos_z=" << e_gen.mu[2] << "\n";

			e_gen.sigma[0] = std::exp(reinterpret_cast<float*>(scales->buffer.get())[idx * 3 + 0]);
			e_gen.sigma[1] = std::exp(reinterpret_cast<float*>(scales->buffer.get())[idx * 3 + 1]);
			e_gen.sigma[2] = std::exp(reinterpret_cast<float*>(scales->buffer.get())[idx * 3 + 2]);

			//float eps = 1e-3f;

			//e_gen.sigma[0] = e_gen.sigma[0] < eps ? eps : e_gen.sigma[0];
			//e_gen.sigma[1] = e_gen.sigma[1] < eps ? eps : e_gen.sigma[1];
			//e_gen.sigma[2] = e_gen.sigma[2] < eps ? eps : e_gen.sigma[2];


			//dump_file << "scale:\n";
			//dump_file << "sx=" << e_gen.sigma[0] << "\n";
			//dump_file << "sy=" << e_gen.sigma[1] << "\n";
			//dump_file << "sz=" << e_gen.sigma[2] << "\n";

			/*std::cout << "Mu:" << std::endl;
			std::cout << e_gen.mu[0] << std::endl;
			std::cout << e_gen.mu[1] << std::endl;
			std::cout << e_gen.mu[2] << std::endl;
			std::cout << "Sigma:" << std::endl;
			std::cout << e_gen.sigma[0] << std::endl;
			std::cout << e_gen.sigma[1] << std::endl;
			std::cout << e_gen.sigma[2] << std::endl;
			std::cout << "Scale^-2" << std::endl;
			std::cout << 1.0 / (e_gen.sigma[0] * e_gen.sigma[0]) << std::endl;
			std::cout << 1.0 / (e_gen.sigma[1] * e_gen.sigma[1]) << std::endl;
			std::cout << 1.0 / (e_gen.sigma[2] * e_gen.sigma[2]) << std::endl;
			std::cout << "------" << std::endl;*/

			float w = reinterpret_cast<float*>(quat_rot->buffer.get())[idx * 4 + 0];
			float x = reinterpret_cast<float*>(quat_rot->buffer.get())[idx * 4 + 1];
			float y = reinterpret_cast<float*>(quat_rot->buffer.get())[idx * 4 + 2];
			float z = reinterpret_cast<float*>(quat_rot->buffer.get())[idx * 4 + 3];

			//dump_file << "rotation:\n";
			//dump_file << "w=" << w << "\n";
			//dump_file << "x=" << x << "\n";
			//dump_file << "y=" << y << "\n";
			//dump_file << "z=" << z << "\n";

			glm::quat q = glm::normalize(glm::quat(w, x, y, z));
			e_gen.rotation = glm::mat4_cast(q);

			//static bool test = true;
			//if (test)
			//{
			//	std::cout << "Rotation matrix:" << std::endl;
			//	for (int row = 0; row < 4; row++) {
			//		for (int col = 0; col < 4; col++) {
			//			std::cout << e_gen.rotation[row][col] << " ";
			//		}
			//		std::cout << std::endl;
			//	}
			//	for (int row = 0; row < 4; row++) {
			//		for (int col = 0; col < 4; col++) {
			//			std::cout << e_gen.rotation[col][row] << " ";
			//		}
			//		std::cout << std::endl;
			//	}
			//	std::cout << e_gen.rotation << std::endl;
			//	test = false;
			//}

			glm::mat4 scale_invatiant_squared = glm::mat4(
				1.0f / (e_gen.sigma[0] * e_gen.sigma[0]), 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f / (e_gen.sigma[1] * e_gen.sigma[1]), 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f / (e_gen.sigma[2] * e_gen.sigma[2]), 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			);

			// e_gen.covariance_invariant = e_gen.rotation * scale_invatiant_squared * glm::transpose(e_gen.rotation);
			e_gen.covariance_invariant = glm::mat4(
				1.0 / (e_gen.sigma[0]), 0.0f, 0.0f, 0.0f,
				0.0f, 1.0 / (e_gen.sigma[1]), 0.0f, 0.0f,
				0.0f, 0.0f, 1.0 / (e_gen.sigma[2]), 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			) * glm::transpose(e_gen.rotation);

			float CO = 0.28209479177387814f;

			float r = reinterpret_cast<float*>(f_dc->buffer.get())[idx * 3 + 0];
			float g = reinterpret_cast<float*>(f_dc->buffer.get())[idx * 3 + 1];
			float b = reinterpret_cast<float*>(f_dc->buffer.get())[idx * 3 + 2];

			e_add.sh_main[0] = std::clamp<float>(0.5f + CO * r, 0.0f, 1.0f);
			e_add.sh_main[1] = std::clamp<float>(0.5f + CO * g, 0.0f, 1.0f);
			e_add.sh_main[2] = std::clamp<float>(0.5f + CO * b, 0.0f, 1.0f);

			//dump_file << "main_color:\n";
			//dump_file << "r=" << e_add.sh_main[0] << "\n";
			//dump_file << "g=" << e_add.sh_main[1] << "\n";
			//dump_file << "b=" << e_add.sh_main[2] << "\n";

			if (f_rest)
			{
				for (std::size_t sh_idx = 0; sh_idx < 45; ++sh_idx)
				{
					//dump_file << "add_color_" << sh_idx / 3 << ":\n";
					//dump_file << "a=" << "\n";
					//dump_file << "b=" << "\n";
					//dump_file << "c=" << "\n";
					e_add.sh_add[sh_idx] = (reinterpret_cast<float*>(f_rest->buffer.get())[idx * 45 + sh_idx]);
				}
			}
			else
			{
				for (std::size_t sh_idx = 0; sh_idx < 45; ++sh_idx)
				{
					e_add.sh_add[sh_idx] = 0.0f;
				}
			}

			float opacity_value = reinterpret_cast<float*>(opacity->buffer.get())[idx];
			opacity_value = (1.0 / (1.0 + std::exp(-opacity_value)));
			e_add.opacity = opacity_value;
			// e_add.opacity = 0.2f;

			gaussians_general.emplace_back(e_gen);
			gaussians_addiotional.emplace_back(e_add);
		}

		dump_file.close();
	}

	void create_cube()
	{
		std::random_device rd;
		std::uniform_real_distribution<float> dis_uniform(0.0f, 1.0f);
		std::mt19937 gen(rd());

		gaussians_general.clear();
		gaussians_addiotional.clear();

		std::size_t n = 10;

		glm::vec3 cube_min(-10, -10, -10);
		glm::vec3 cube_max(10, 10, 10);
		glm::vec3 length = cube_max - cube_min;
		glm::vec3 step = length / float(n - 1);

		struct Face {
			int fixed_axis;
			float fixed_value;
		};

		std::vector<Face> faces = {
			{0, cube_min.x},  // x = -10
			{0, cube_max.x},  // x =  10
			{1, cube_min.y},  // y = -10
			{1, cube_max.y},  // y =  10
			{2, cube_min.z},  // z = -10
			{2, cube_max.z}   // z =  10
		};

		for (const auto& face : faces)
		{
			for (std::size_t i = 0; i < n; ++i)
			{
				for (std::size_t j = 0; j < n; ++j)
				{
					// ������ ����������
					glm::vec3 coord;

					// ��� ��������� ��� �������� �������� i � j
					int free_axis1 = (face.fixed_axis + 1) % 3;
					int free_axis2 = (face.fixed_axis + 2) % 3;

					coord[face.fixed_axis] = face.fixed_value;
					coord[free_axis1] = cube_min[free_axis1] + step[free_axis1] * float(i);
					coord[free_axis2] = cube_min[free_axis2] + step[free_axis2] * float(j);

					// ������ ���������
					Ellipsoid::EllipsoidGeneral e_gen;
					Ellipsoid::EllipsoidAddtitional e_add;

					e_gen.mu = glm::vec4(coord, 0);
					e_gen.sigma = glm::vec4(0.5f, 0.5f, 0.5f, 0);
					e_gen.rotation = glm::mat4(1.0f);

					glm::mat4 scale_invatiant_squared = glm::mat4(
						1.0f / (e_gen.sigma[0] * e_gen.sigma[0]), 0.0f, 0.0f, 0.0f,
						0.0f, 1.0f / (e_gen.sigma[1] * e_gen.sigma[1]), 0.0f, 0.0f,
						0.0f, 0.0f, 1.0f / (e_gen.sigma[2] * e_gen.sigma[2]), 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f
					);

					e_gen.covariance_invariant = scale_invatiant_squared;

					//e_add.sh_main[0] = 1.78f;
					//e_add.sh_main[1] = 1.78f;
					//e_add.sh_main[2] = 1.78f;

					e_add.sh_main[0] = dis_uniform(gen);
					e_add.sh_main[1] = dis_uniform(gen);
					e_add.sh_main[2] = dis_uniform(gen);

					for (std::size_t sh_idx = 0; sh_idx < 45; ++sh_idx)
					{
						e_add.sh_add[sh_idx] = 0.0f;
					}

					e_add.opacity = 1.0f;

					gaussians_general.emplace_back(e_gen);
					gaussians_addiotional.emplace_back(e_add);
				}
			}
		}

		for (const auto& face : faces)
		{
			for (std::size_t i = 0; i < n; ++i)
			{
				for (std::size_t j = 0; j < n; ++j)
				{

					glm::vec3 coord;


					int free_axis1 = (face.fixed_axis + 1) % 3;
					int free_axis2 = (face.fixed_axis + 2) % 3;

					coord[face.fixed_axis] = face.fixed_value;
					coord[free_axis1] = cube_min[free_axis1] + step[free_axis1] * float(i);
					coord[free_axis2] = cube_min[free_axis2] + step[free_axis2] * float(j);


					Ellipsoid::EllipsoidGeneral e_gen;
					Ellipsoid::EllipsoidAddtitional e_add;

					e_gen.mu = glm::vec4(coord + glm::vec3(50.0f, 50.0f, 0.0f), 0);
					e_gen.sigma = glm::vec4(0.5f, 0.5f, 0.5f, 0);
					e_gen.rotation = glm::mat4(1.0f);

					glm::mat4 scale_invatiant_squared = glm::mat4(
						1.0f / (e_gen.sigma[0] * e_gen.sigma[0]), 0.0f, 0.0f, 0.0f,
						0.0f, 1.0f / (e_gen.sigma[1] * e_gen.sigma[1]), 0.0f, 0.0f,
						0.0f, 0.0f, 1.0f / (e_gen.sigma[2] * e_gen.sigma[2]), 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f
					);

					e_gen.covariance_invariant = scale_invatiant_squared;

					//e_add.sh_main[0] = 1.78f;
					//e_add.sh_main[1] = 1.78f;
					//e_add.sh_main[2] = 1.78f;

					e_add.sh_main[0] = dis_uniform(gen);
					e_add.sh_main[1] = dis_uniform(gen);
					e_add.sh_main[2] = dis_uniform(gen);

					for (std::size_t sh_idx = 0; sh_idx < 45; ++sh_idx)
					{
						e_add.sh_add[sh_idx] = 0.0f;
					}

					e_add.opacity = 1.0f;

					gaussians_general.emplace_back(e_gen);
					gaussians_addiotional.emplace_back(e_add);
				}
			}
		}

		std::cout << "Created cube with " << gaussians_general.size() << " gaussians." << std::endl;
	}

	void create_line()
	{
		std::random_device rd;
		std::uniform_real_distribution<float> dis_uniform(0.0f, 1.0f);
		std::mt19937 gen(rd());

		gaussians_general.clear();
		gaussians_addiotional.clear();

		std::size_t n = 10;

		float step = 10.0f;

		for (std::size_t j = 0; j < n; ++j)
		{

			Ellipsoid::EllipsoidGeneral e_gen;
			Ellipsoid::EllipsoidAddtitional e_add;

			e_gen.mu = glm::vec4(glm::vec3(0.0, 0.0, j * step), 0);
			e_gen.sigma = glm::vec4(0.5f, 0.5f, 0.5f, 0);
			e_gen.rotation = glm::mat4(1.0f);

			glm::mat4 scale_invatiant_squared = glm::mat4(
				1.0f / (e_gen.sigma[0] * e_gen.sigma[0]), 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f / (e_gen.sigma[1] * e_gen.sigma[1]), 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f / (e_gen.sigma[2] * e_gen.sigma[2]), 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			);

			e_gen.covariance_invariant = scale_invatiant_squared;

			//e_add.sh_main[0] = 1.78f;
			//e_add.sh_main[1] = 1.78f;
			//e_add.sh_main[2] = 1.78f;

			if (j % 3 == 0)
			{
				e_add.sh_main[0] = -1.78f;
				e_add.sh_main[1] = 1.78f;
				e_add.sh_main[2] = -1.78f;
			}
			else if (j % 3 == 1)
			{
				e_add.sh_main[0] = 1.78f;
				e_add.sh_main[1] = -1.78f;
				e_add.sh_main[2] = -1.78f;
			}
			else
			{
				e_add.sh_main[0] = -1.78f;
				e_add.sh_main[1] = -1.78f;
				e_add.sh_main[2] = 1.78f;
			}

			for (std::size_t sh_idx = 0; sh_idx < 45; ++sh_idx)
			{
				e_add.sh_add[sh_idx] = 0.0f;
			}

			e_add.opacity = 1.0f;

			gaussians_general.emplace_back(e_gen);
			gaussians_addiotional.emplace_back(e_add);
		}

		std::cout << "Created cube with " << gaussians_general.size() << " gaussians." << std::endl;
	}


	std::vector<Ellipsoid::EllipsoidGeneral>& get_gaussians_general() { return gaussians_general; }
	std::vector<Ellipsoid::EllipsoidAddtitional>& get_gaussians_additional() { return gaussians_addiotional; }

	std::size_t get_gaussians_count() const { return gaussians_general.size(); }

private:

	glm::mat4 get_rotation_matrix_from_quaternion(const glm::vec4& quaternion)
	{
		glm::quat q(quaternion);
		return glm::mat4_cast(q);
	}

};


#endif // !SCENE_LOADER_H
