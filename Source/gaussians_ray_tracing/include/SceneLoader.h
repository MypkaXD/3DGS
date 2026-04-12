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
	std::vector<Ellipsoid::EllipsoidAdditional> gaussians_additional;
public:

	SceneLoader() { }

	void create_random_scene(const std::size_t n)
	{
		std::random_device rd;
		std::mt19937 gen(rd());

		std::uniform_real_distribution<float> dis(-1.0f, 1.0f);

		gaussians_general.clear();
		gaussians_additional.clear();

		gaussians_general.reserve(n);
		gaussians_additional.reserve(n);

		Ellipsoid::Q = 1.0f;

		for (std::size_t idx = 0; idx < n; ++idx)
		{
			float x = dis(gen);
			float y = dis(gen);
			float z = dis(gen);
			float w = dis(gen);

			float norm = std::sqrt(x * x + y * y + z * z + w * w);

			Ellipsoid::EllipsoidAdditional e_add;

			e_add.sh_main[0] = dis(gen);
			e_add.sh_main[1] = dis(gen);
			e_add.sh_main[2] = dis(gen);

			for (std::size_t idx_sh = 0; idx_sh < 45; ++idx_sh)
			{
				e_add.sh_rest[idx_sh] = dis(gen);
			}

			e_add.opacity = dis(gen);

			glm::mat3 rotation = calculate_rotation_matrix(x / norm, y / norm, z / norm, w / norm); // rotation
			glm::vec3 sigma = glm::vec3(dis(gen) + 1.0f, dis(gen) + 1.0f, dis(gen) + 1.0f); // sigma
			glm::vec3 mu = glm::vec3(dis(gen) * 1000, dis(gen) * 1000, dis(gen) * 1000); // mu
			
			glm::mat3 inverse_sigma_squared = glm::mat3(
				1.0f / (sigma.x * sigma.x), 0.0f, 0.0f,
				0.0f, 1.0f / (sigma.y * sigma.y), 0.0f,
				0.0f, 0.0f, 1.0f / (sigma.z * sigma.z)
			);

			gaussians_general.emplace_back(
				mu, // mu
				sigma, // sigma
				rotation, // rotation
				rotation * inverse_sigma_squared * glm::transpose(rotation) // inverse covariance
			);
			gaussians_additional.emplace_back(e_add);
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

		Ellipsoid::Q = 1.0f;

		for (std::size_t idx = 0; idx < xyz->count; ++idx)
		{
			Ellipsoid::EllipsoidAdditional e_add;

			e_add.sh_main[0] = (reinterpret_cast<float*>(f_dc->buffer.get())[idx * 3 + 0]);
			e_add.sh_main[1] = (reinterpret_cast<float*>(f_dc->buffer.get())[idx * 3 + 1]);
			e_add.sh_main[2] = (reinterpret_cast<float*>(f_dc->buffer.get())[idx * 3 + 2]);

			if (f_rest)
			{
				for (std::size_t sh_idx = 0; sh_idx < 45; ++sh_idx)
				{
					e_add.sh_rest[sh_idx] = 1.0f;
				}
			}

			e_add.opacity = reinterpret_cast<float*>(opacity->buffer.get())[idx];

			glm::mat3 rotation = calculate_rotation_matrix(
				reinterpret_cast<float*>(quat_rot->buffer.get())[idx * 4 + 0],
				reinterpret_cast<float*>(quat_rot->buffer.get())[idx * 4 + 1],
				reinterpret_cast<float*>(quat_rot->buffer.get())[idx * 4 + 2],
				reinterpret_cast<float*>(quat_rot->buffer.get())[idx * 4 + 3]
			); // rotation
			glm::vec3 sigma = glm::vec3(reinterpret_cast<float*>(scales->buffer.get())[idx * 3 + 0], reinterpret_cast<float*>(scales->buffer.get())[idx * 3 + 1], reinterpret_cast<float*>(scales->buffer.get())[idx * 3 + 2]); // sigma
			glm::vec3 mu = glm::vec3(reinterpret_cast<float*>(xyz->buffer.get())[idx * 3 + 0], reinterpret_cast<float*>(xyz->buffer.get())[idx * 3 + 1], reinterpret_cast<float*>(xyz->buffer.get())[idx * 3 + 2]); // mu

			glm::mat3 inverse_sigma_squared = glm::mat3(
				1.0f / (sigma.x * sigma.x), 0.0f, 0.0f,
				0.0f, 1.0f / (sigma.y * sigma.y), 0.0f,
				0.0f, 0.0f, 1.0f / (sigma.z * sigma.z)
			);

			gaussians_general.emplace_back(
				mu,
				sigma,
				rotation,
				rotation* inverse_sigma_squared* glm::transpose(rotation)
			);
			gaussians_additional.emplace_back(e_add);
		}
	}

	std::vector<Ellipsoid::EllipsoidGeneral>& get_gaussians_general() { return gaussians_general; }
	std::vector<Ellipsoid::EllipsoidAdditional>& get_gaussians_additional() { return gaussians_additional; }

private:

	glm::mat3 calculate_rotation_matrix(const float x, const float y, const float z, const float w)
	{
		return glm::mat3{
			{1 - 2 * (y * y + z * z), 2 * (x * y - z * w), 2 * (x * z + y * w)},
			{2 * (x * y + z * w), 1 - 2 * (x * x + z * z), 2 * (y * z - x * w)},
			{2 * (x * z - y * w), 2 * (y * z + x * w), 1 - 2 * (x * x + y * y)}
		};
	}

};


#endif // !SCENE_LOADER_H
