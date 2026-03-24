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
	std::vector<Ellipsoid> gaussians;
public:

	SceneLoader() { }

	void create_random_scene(const std::size_t n)
	{
		std::random_device rd;
		std::mt19937 gen(rd());

		std::uniform_int_distribution<int> dis_mu(-100, 100);
		std::uniform_real_distribution<float> dis_sigma(1, 5);
		std::normal_distribution<float> dis_normal(0.0f, 1.0f);
		std::uniform_real_distribution<float> dis_uniform_0_to_1(0.0f, 1.0f);

		gaussians.clear();
		gaussians.reserve(n);

		for (std::size_t idx = 0; idx < n; ++idx)
		{
			Ellipsoid e;

			e.mu[0] = dis_mu(gen);
			e.mu[1] = dis_mu(gen);
			e.mu[2] = dis_mu(gen);

			e.sigma[0] = dis_sigma(gen);
			e.sigma[1] = dis_sigma(gen);
			e.sigma[2] = dis_sigma(gen);

			float x = dis_normal(gen);
			float y = dis_normal(gen);
			float z = dis_normal(gen);
			float w = dis_normal(gen);

			float norm = std::sqrt(x * x + y * y + z * z + w * w);

			e.quaternion[0] = x / norm;
			e.quaternion[1] = y / norm;
			e.quaternion[2] = z / norm;
			e.quaternion[3] = w / norm;

			e.sh_dc[0] = 0;
			e.sh_dc[1] = 0;
			e.sh_dc[2] = 0;

			for (std::size_t idx_sh = 0; idx_sh < 45; ++idx_sh)
			{
				e.sh_rest[idx_sh] = dis_uniform_0_to_1(gen);
			}

			e.Q = 1.0f;

			gaussians.emplace_back(e);
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

		for (std::size_t idx = 0; idx < xyz->count; ++idx)
		{
			Ellipsoid e;

			e.mu[0] = reinterpret_cast<float*>(xyz->buffer.get())[idx * 3 + 0];
			e.mu[1] = reinterpret_cast<float*>(xyz->buffer.get())[idx * 3 + 1];
			e.mu[2] = reinterpret_cast<float*>(xyz->buffer.get())[idx * 3 + 2];

			e.sigma[0] = reinterpret_cast<float*>(scales->buffer.get())[idx * 3 + 0];
			e.sigma[1] = reinterpret_cast<float*>(scales->buffer.get())[idx * 3 + 1];
			e.sigma[2] = reinterpret_cast<float*>(scales->buffer.get())[idx * 3 + 2];

			e.quaternion[0] = reinterpret_cast<float*>(quat_rot->buffer.get())[idx * 4 + 0];
			e.quaternion[1] = reinterpret_cast<float*>(quat_rot->buffer.get())[idx * 4 + 1];
			e.quaternion[2] = reinterpret_cast<float*>(quat_rot->buffer.get())[idx * 4 + 2];
			e.quaternion[3] = reinterpret_cast<float*>(quat_rot->buffer.get())[idx * 4 + 3];

			e.sh_dc[0] = (reinterpret_cast<float*>(f_dc->buffer.get())[idx * 3 + 0]);
			e.sh_dc[1] = (reinterpret_cast<float*>(f_dc->buffer.get())[idx * 3 + 1]);
			e.sh_dc[2] = (reinterpret_cast<float*>(f_dc->buffer.get())[idx * 3 + 2]);

			for (std::size_t sh_idx = 0; sh_idx < 15; ++sh_idx)
			{
				if (f_rest)
				{
					if (idx == 0)
					{
						e.sh_rest[sh_idx] = reinterpret_cast<float*>(f_rest->buffer.get())[idx * 15 + sh_idx];
						std::cout << e.sh_rest[sh_idx] << std::endl;
					}
				}
			}

			e.opacity = reinterpret_cast<float*>(opacity->buffer.get())[idx];
			e.Q = 1.0f;

			gaussians.emplace_back(e);
		}
	}

	std::vector<Ellipsoid>& get_gaussians() { return gaussians; }
};


#endif // !SCENE_LOADER_H
