#include <iostream>
#include <random>
#include <array>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "tinyply.h"

#include <Shader.h>
#include <Camera.h>
#include <Ellipsoid.h>
#include <AABB.h>

#include <bvh/v2/bvh.h>
#include <bvh/v2/vec.h>
#include <bvh/v2/bbox.h>
#include <bvh/v2/default_builder.h>
#include <bvh/v2/node.h>

using Scalar = float;
using Vec3 = bvh::v2::Vec<Scalar, 3>;
using BBox = bvh::v2::BBox<Scalar, 3>;
using Node = bvh::v2::Node<Scalar, 3>;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


void read_gaussian_splat_ply(const std::string& filepath, std::vector<Ellipsoid>& gaussians)
{

	std::ifstream file_stream(filepath, std::ios::binary);
	if (!file_stream) throw std::runtime_error("file_stream failed to open " + filepath);

	file_stream.seekg(0, std::ios::end);
	const float size_mb = file_stream.tellg() * float(1e-6);
	file_stream.seekg(0, std::ios::beg);

	tinyply::PlyFile splat_ply;
	splat_ply.parse_header(file_stream);

	std::cout << "\t[ply_header] Type: " << (splat_ply.is_binary_file() ? "binary" : "ascii") << std::endl;
	for (const auto& c : splat_ply.get_comments()) std::cout << "\t[ply_header] Comment: " << c << std::endl;
	for (const auto& c : splat_ply.get_info()) std::cout << "\t[ply_header] Info: " << c << std::endl;

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
		float mu_x = reinterpret_cast<float*>(xyz->buffer.get())[idx * 3 + 0];
		float mu_y = reinterpret_cast<float*>(xyz->buffer.get())[idx * 3 + 1];
		float mu_z = reinterpret_cast<float*>(xyz->buffer.get())[idx * 3 + 2];
		float sigma_x = reinterpret_cast<float*>(scales->buffer.get())[idx * 3 + 0];
		float sigma_y = reinterpret_cast<float*>(scales->buffer.get())[idx * 3 + 1];
		float sigma_z = reinterpret_cast<float*>(scales->buffer.get())[idx * 3 + 2];
		float quaternion_x = reinterpret_cast<float*>(quat_rot->buffer.get())[idx * 4 + 0];
		float quaternion_y = reinterpret_cast<float*>(quat_rot->buffer.get())[idx * 4 + 1];
		float quaternion_z = reinterpret_cast<float*>(quat_rot->buffer.get())[idx * 4 + 2];
		float quaternion_w = reinterpret_cast<float*>(quat_rot->buffer.get())[idx * 4 + 3];
		gaussians.emplace_back(std::array<float, 3>{ mu_x, mu_y, mu_z }, std::array<float, 3>{sigma_x, sigma_y, sigma_z}, std::array<float, 4>{quaternion_x, quaternion_y, quaternion_z, quaternion_w}, std::sqrt(7.91));
	}
}

void generate_gaussians(std::vector<Ellipsoid>& data, std::size_t n)
{
	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_int_distribution<int> dis_mu(-100, 100);
	std::uniform_real_distribution<float> dis_sigma(1, 5);
	std::normal_distribution<float> dis_normal(0.0f, 1.0f);

	data.clear();
	data.reserve(n);

	for (std::size_t idx = 0; idx < n; ++idx)
	{
		float sigma_x = dis_sigma(gen);
		float sigma_y = dis_sigma(gen);
		float sigma_z = dis_sigma(gen);

		float mu_x = dis_mu(gen);
		float mu_y = dis_mu(gen);
		float mu_z = dis_mu(gen);

		float x = dis_normal(gen);
		float y = dis_normal(gen);
		float z = dis_normal(gen);
		float w = dis_normal(gen);

		float norm = std::sqrt(x * x + y * y + z * z + w * w);

		float quaternion_x = x / norm;
		float quaternion_y = y / norm;
		float quaternion_z = z / norm;
		float quaternion_w = w / norm;

		data.emplace_back(std::array<float, 3>{ mu_x, mu_y, mu_z }, std::array<float, 3>{sigma_x, sigma_y, sigma_z}, std::array<float, 4>{quaternion_x, quaternion_y, quaternion_z, quaternion_w}, std::sqrt(7.91));
	}
}

void generate_AABB(std::vector<Ellipsoid>& gaussians, std::vector<AABB>& aabb)
{

	aabb.clear();
	aabb.reserve(gaussians.size());

	for (std::size_t idx = 0; idx < gaussians.size(); ++idx)
	{

		float x = gaussians[idx].quaternion[0];
		float y = gaussians[idx].quaternion[1];
		float z = gaussians[idx].quaternion[2];
		float w = gaussians[idx].quaternion[3];

		float xx = x * x;
		float xy = x * y;
		float xz = x * z;
		float xw = x * w;

		float yy = y * y;
		float yz = y * z;
		float yw = y * w;

		float zz = z * z;
		float zw = z * w;

		glm::mat3 R = glm::mat3{
			1.0 - 2.0 * (yy + zz), 2.0 * (xy - zw), 2.0 * (xz + yw),
			2.0 * (xy + zw), 1.0 - 2.0 * (xx + zz), 2.0 * (yz - xw),
			2.0 * (xz - yw), 2.0 * (yz + xw), 1.0 - 2.0 * (xx + yy)
		};

		glm::vec3 mu = glm::vec3{ gaussians[idx].mu[0], gaussians[idx].mu[1], gaussians[idx].mu[2] };
		glm::vec3 sigma = glm::vec3{ gaussians[idx].sigma[0], gaussians[idx].sigma[1], gaussians[idx].sigma[2] };

		aabb.emplace_back(R, mu, sigma, 7.91);
	}
}

BBox to_bbox(const AABB& aabb)
{
	Vec3 min = Vec3(
		aabb.center.x - aabb.extent.x,
		aabb.center.y - aabb.extent.y,
		aabb.center.z - aabb.extent.z
	);

	Vec3 max = Vec3(
		aabb.center.x + aabb.extent.x,
		aabb.center.y + aabb.extent.y,
		aabb.center.z + aabb.extent.z
	);

	return BBox(min, max);
}

int main()
{

	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1280, 720, "Hello, World!", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glEnable(GL_DEPTH_TEST);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");

	std::vector<Ellipsoid> gaussians;
	// read_gaussian_splat_ply("C:\\dev\\Gaussian_Splatting\\Splatshop\\splatmodels\\splats\\point_cloud.ply", gaussians);
	generate_gaussians(gaussians, 10);

	std::vector<AABB> aabb;
	generate_AABB(gaussians, aabb);

	std::vector<BBox> bboxes;
	std::vector<Vec3> centers;

	bboxes.reserve(aabb.size());
	centers.reserve(aabb.size());

	for (const auto& box : aabb)
	{
		BBox bbox = to_bbox(box);
		bboxes.push_back(bbox);
		centers.push_back(bbox.get_center());
	}

	typename bvh::v2::DefaultBuilder<Node>::Config config;
	config.quality = bvh::v2::DefaultBuilder<Node>::Quality::High;
	config.min_leaf_size = 4;
	config.max_leaf_size = 8;
	
	auto bvh = bvh::v2::DefaultBuilder<Node>::build(bboxes, centers, config);

	aabb.clear();
	aabb.reserve(bvh.nodes.size());

	for (std::size_t idx = 0; idx < bvh.nodes.size(); ++idx)
	{
		auto box = bvh.nodes[idx].get_bbox();

		// Get center and extent correctly
		glm::vec3 center(
			box.min[0] + (box.max[0] - box.min[0]) * 0.5f,
			box.min[1] + (box.max[1] - box.min[1]) * 0.5f,
			box.min[2] + (box.max[2] - box.min[2]) * 0.5f
		);

		glm::vec3 extent(
			(box.max[0] - box.min[0]) * 0.5f,
			(box.max[1] - box.min[1]) * 0.5f,
			(box.max[2] - box.min[2]) * 0.5f
		);

		aabb.emplace_back(center, extent);
	}

	UV uv(20, 20, 0.0f, 2.0f * M_PI, 0.0f, M_PI);
	uv.generate();

	float cube_vertices[] = {
		-1,-1,-1,
		 1,-1,-1,
		 1, 1,-1,
		-1, 1,-1,
		-1,-1, 1,
		 1,-1, 1,
		 1, 1, 1,
		-1, 1, 1
	};

	unsigned int cube_indices[] = {
		0,1, 1,2, 2,3, 3,0,
		4,5, 5,6, 6,7, 7,4,
		0,4, 1,5, 2,6, 3,7
	};

	unsigned int VAO_Gaussians;
	unsigned int VBO_UV, VBO_Gaussians;
	unsigned int EBO_Gaussians;

	unsigned int VAO_AABB;
	unsigned int VBO_AABB, VBO_CUBE;
	unsigned int EBO_AABB;

	glGenVertexArrays(1, &VAO_Gaussians);
	glGenBuffers(1, &EBO_Gaussians);
	glGenBuffers(1, &VBO_Gaussians);
	glGenBuffers(1, &VBO_UV);

	glGenVertexArrays(1, &VAO_AABB);
	glGenBuffers(1, &EBO_AABB);
	glGenBuffers(1, &VBO_AABB);
	glGenBuffers(1, &VBO_CUBE);


	glBindVertexArray(VAO_Gaussians);

	// UV
	// for setting buffer for vertex of UV Space
	glBindBuffer(GL_ARRAY_BUFFER, VBO_UV);
	glBufferData(GL_ARRAY_BUFFER, uv.get_vertices().size() * sizeof(float), uv.get_vertices().data(), GL_STATIC_DRAW);

	// for setting buffer for indexes of UV
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_Gaussians);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, uv.get_indices().size() * sizeof(unsigned int), uv.get_indices().data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	// Gaussians
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Gaussians);
	glBufferData(GL_ARRAY_BUFFER, gaussians.size() * sizeof(Ellipsoid), gaussians.data(), GL_STATIC_DRAW);

	// Sigma
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Ellipsoid), (void*)0);
	glEnableVertexAttribArray(1);

	// Mu
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Ellipsoid), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// Quaternion
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Ellipsoid), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(3);

	// Q
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Ellipsoid), (void*)(10 * sizeof(float)));
	glEnableVertexAttribArray(4);

	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);


	// AABB
	glBindVertexArray(VAO_AABB);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_CUBE);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_AABB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_AABB);
	glBufferData(GL_ARRAY_BUFFER, aabb.size() * sizeof(AABB), aabb.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AABB), (void*)0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(AABB), (void*)(sizeof(glm::vec3)));
	glEnableVertexAttribArray(2);

	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);

	glBindVertexArray(0);

	Shader shader_gaussians;
	shader_gaussians.load_shaders(std::make_pair("C:\\dev\\Gaussian_Splatting\\3DGS\\Source\\gaussians_viewer\\shaders\\vertex_shader_gaussians.vert", GL_VERTEX_SHADER), std::make_pair("C:\\dev\\Gaussian_Splatting\\3DGS\\Source\\gaussians_viewer\\shaders\\fragment_shader.frag", GL_FRAGMENT_SHADER));
	shader_gaussians.link();

	Shader shader_aabb;
	shader_aabb.load_shaders(std::make_pair("C:\\dev\\Gaussian_Splatting\\3DGS\\Source\\gaussians_viewer\\shaders\\vertex_shader_aabb.vert", GL_VERTEX_SHADER), std::make_pair("C:\\dev\\Gaussian_Splatting\\3DGS\\Source\\gaussians_viewer\\shaders\\fragment_shader.frag", GL_FRAGMENT_SHADER));
	shader_aabb.link();

	bool draw_ellipsoid_by_lines = false;
	bool draw_AABB = true;

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		glm::mat4 view = camera.get_view_matrix(); // camera matrix
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f); // projection matrix
		glm::mat4 model = glm::mat4(1.0f); // model matrix

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader_gaussians.use();
		glBindVertexArray(VAO_Gaussians);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_Gaussians);

		glUniformMatrix4fv(glGetUniformLocation(shader_gaussians.get_id(), "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shader_gaussians.get_id(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(shader_gaussians.get_id(), "model"), 1, GL_FALSE, glm::value_ptr(model));

		// draw ellipsoids
		{
			if (draw_ellipsoid_by_lines)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			glDrawElementsInstanced(GL_TRIANGLES, uv.get_indices().size(), GL_UNSIGNED_INT, 0, gaussians.size());
		}

		// drwa AABB
		{
			if (draw_AABB) {

				shader_aabb.use();
				glBindVertexArray(VAO_AABB);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_AABB);

				glUniformMatrix4fv(glGetUniformLocation(shader_aabb.get_id(), "view"), 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(glGetUniformLocation(shader_aabb.get_id(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
				glUniformMatrix4fv(glGetUniformLocation(shader_aabb.get_id(), "model"), 1, GL_FALSE, glm::value_ptr(model));

				glDrawElementsInstanced(
					GL_LINES,
					24,
					GL_UNSIGNED_INT,
					0,
					aabb.size()
				);
			}
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Settings");
		ImGui::Checkbox("Draw ellipsoids by lines", &draw_ellipsoid_by_lines);
		ImGui::Checkbox("Draw AABB", &draw_AABB);
		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO_Gaussians);
	glDeleteVertexArrays(1, &VAO_AABB);
	glDeleteBuffers(1, &VBO_Gaussians);
	glDeleteBuffers(1, &VBO_UV);
	glDeleteBuffers(1, &VBO_AABB);
	glDeleteBuffers(1, &VBO_CUBE);
	glDeleteBuffers(1, &EBO_Gaussians);
	glDeleteBuffers(1, &EBO_AABB);
	shader_gaussians.~Shader();
	shader_aabb.~Shader();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();

	return 0;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.process_keyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.process_keyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.process_keyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.process_keyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.process_mouse_movement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.process_mouse_scroll(static_cast<float>(yoffset));
}