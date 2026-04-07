#ifndef APP_H
#define APP_H

#include <iostream>
#include <chrono>

#define _USE_MATH_DEFINES
#include <math.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <UV.h>
#include <SceneLoader.h>
#include <AABB.h>
#include <BVH.h>
#include <Camera.h>
#include <Shader.h>

class App
{
private:

	static unsigned int m_width;
	static unsigned int m_height;


	GLFWwindow* m_window = nullptr;

	SceneLoader m_loader;

	bool is_draw_gaussians_solid = false;
	bool m_sucessfully_initialized = false; // to check if we need to clean up resources in destructor
	
	bool m_draw_aabb_initialize = false;
	bool m_draw_aabb = false;

	bool m_draw_bvh_initialize = false;
	bool m_draw_bvh = false;

	// Buffers of gaussians
	unsigned int m_VAO_Gaussians;
	unsigned int m_SSBO_Gaussians;

	// Buffers for gaussians Solid
	unsigned int m_VBO_UV;
	unsigned int m_EBO_UV;

	// Buffers for AABB
	unsigned int m_VAO_BOX_AABB;
	unsigned int m_VBO_BOX_AABB;
	unsigned int m_EBO_BOX_AABB;
	unsigned int m_VBO_AABB;
	
	// Buffers for BVH
	unsigned int m_VAO_BOX_BVH;
	unsigned int m_VBO_BOX_BVH;
	unsigned int m_EBO_BOX_BVH;
	unsigned int m_VBO_BVH;

	// Shaders
	Shader m_shader_gaussians;
	Shader m_shader_aabb;
	Shader m_shader_light_source;

	// Camera settings
	static Camera m_camera;
	static std::pair<float, float> m_last_coords;
	static bool m_first_mouse;
	static bool m_mouse_press;

	// Timing
	static float m_delta_time;	// time between current frame and last frame
	static float m_last_frame;

	UV uv;

	std::vector<AABB> m_bvh;

	float m_gaussian_Q = 1.0f;

public:

	App()
	{
	}

	~App()
	{
		if (m_sucessfully_initialized)
		{
			glDeleteVertexArrays(1, &m_VAO_Gaussians);

			glDeleteBuffers(1, &m_SSBO_Gaussians);
			glDeleteBuffers(1, &m_VBO_UV);
			glDeleteBuffers(1, &m_EBO_UV);

			if (m_draw_aabb_initialize)
			{
				glDeleteVertexArrays(1, &m_VAO_BOX_AABB);
				glDeleteBuffers(1, &m_EBO_BOX_AABB);
				glDeleteBuffers(1, &m_VBO_BOX_AABB);
				glDeleteBuffers(1, &m_VBO_AABB);
			}

			if (m_draw_bvh_initialize)
			{
				glDeleteVertexArrays(1, &m_VAO_BOX_BVH);
				glDeleteBuffers(1, &m_EBO_BOX_BVH);
				glDeleteBuffers(1, &m_VBO_BOX_BVH);
				glDeleteBuffers(1, &m_VBO_BVH);
			}

			m_shader_gaussians.~Shader();
			m_shader_aabb.~Shader();
			m_shader_light_source.~Shader();
		}

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		glfwTerminate();

	}

	bool init()
	{
		init_camera();

		if (init_opengl() == false)
			return false;
		if (init_imgui() == false)
			return false;

		glEnable(GL_DEPTH_TEST);

		load_shaders();

		init_gaussians();

		uv = UV(100, 10, 0.0f, 2.0f * M_PI, 0.0f, M_PI);
		uv.generate();

		std::vector<AABB> aabb;
		generate_AABB_from_gaussians(m_loader.get_gaussians(), aabb);

		create_bvh(aabb, m_bvh);

		glBindVertexArray(m_VAO_Gaussians);

		glGenBuffers(1, &m_VBO_UV);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO_UV);
		glBufferData(GL_ARRAY_BUFFER, uv.get_vertices().size() * sizeof(float), uv.get_vertices().data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &m_EBO_UV);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_UV);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, uv.get_indices().size() * sizeof(unsigned int), uv.get_indices().data(), GL_STATIC_DRAW);

		glBindVertexArray(0);

		m_sucessfully_initialized = true;

		return true;
	}

	void main_loop()
	{

		std::array<float, 3> bvh_color = { 1.0f, 1.0f, 1.0f};
		std::array<float, 3> aabb_color = { 1.0f, 1.0f, 1.0f};

		// ambient params
		std::array<float, 3> ambient_color = { 1.0f, 1.0f, 1.0f};
		float ambient_strength = 0.1f;

		// diffuse params
		std::array<float, 3> light_pos = { 100.0f * std::sin(0) * std::cos(0), 100.0f * std::sin(0) * std::sin(0), 0.0f};
		std::array<float, 3> light_color = { 1.0f, 1.0f, 1.0f };

		bool draw_points = false;

		auto start_time = std::chrono::high_resolution_clock::now();
		auto end_time = std::chrono::high_resolution_clock::now();
		auto fps = 0.0f;

		while (!glfwWindowShouldClose(m_window))
		{

			start_time = std::chrono::high_resolution_clock::now();

			float currentFrame = static_cast<float>(glfwGetTime());
			m_delta_time = currentFrame - m_last_frame;
			m_last_frame = currentFrame;

			light_pos[0] = 100.0f * std::sin(currentFrame) * std::cos(currentFrame);
			light_pos[1] = 100.0f * std::sin(currentFrame) * std::sin(currentFrame);

			process_input(m_window);

			glm::mat4 view = m_camera.get_view_matrix(); // camera matrix
			glm::mat4 projection = glm::perspective(glm::radians(m_camera.Zoom), (float)m_width / (float)m_height, 0.1f, 1000.0f); // projection matrix
			glm::mat4 model = glm::mat4(1.0f); // model matrix

			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// draw ellipsoids
			{
				m_shader_gaussians.use();

				glUniformMatrix4fv(glGetUniformLocation(m_shader_gaussians.get_id(), "view"), 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(glGetUniformLocation(m_shader_gaussians.get_id(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
				glUniformMatrix4fv(glGetUniformLocation(m_shader_gaussians.get_id(), "model"), 1, GL_FALSE, glm::value_ptr(model));

				glUniform3f(glGetUniformLocation(m_shader_gaussians.get_id(), "ambient_color"), ambient_color[0], ambient_color[1], ambient_color[2]);
				glUniform1f(glGetUniformLocation(m_shader_gaussians.get_id(), "ambient_strength"), ambient_strength);
				
				glUniform3f(glGetUniformLocation(m_shader_gaussians.get_id(), "diffuse_pos"), light_pos[0], light_pos[1], light_pos[2]);
				glUniform3f(glGetUniformLocation(m_shader_gaussians.get_id(), "diffuse_color"), light_color[0], light_color[1], light_color[2]);
				
				glUniform3f(glGetUniformLocation(m_shader_gaussians.get_id(), "camera_pos"), m_camera.Position[0], m_camera.Position[1], m_camera.Position[2]);

				glBindVertexArray(m_VAO_Gaussians);
				if (draw_points)
				{
					glDrawElementsInstanced(GL_POINTS, uv.get_indices().size(), GL_UNSIGNED_INT, 0, m_loader.get_gaussians().size());
				}
				else
				{
					glDrawElementsInstanced(GL_TRIANGLES, uv.get_indices().size(), GL_UNSIGNED_INT, 0, m_loader.get_gaussians().size());
				}
			}

			// draw light source
			{
				m_shader_light_source.use();

				glUniformMatrix4fv(glGetUniformLocation(m_shader_light_source.get_id(), "view"), 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(glGetUniformLocation(m_shader_light_source.get_id(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
				glUniformMatrix4fv(glGetUniformLocation(m_shader_light_source.get_id(), "model"), 1, GL_FALSE, glm::value_ptr(model));

				glUniform3f(glGetUniformLocation(m_shader_light_source.get_id(), "diffuse_pos"), light_pos[0], light_pos[1], light_pos[2]);
				glUniform3f(glGetUniformLocation(m_shader_light_source.get_id(), "diffuse_color"), light_color[0], light_color[1], light_color[2]);

				glBindVertexArray(m_VAO_Gaussians);
				if (draw_points)
				{
					glDrawElementsInstanced(GL_POINTS, uv.get_indices().size(), GL_UNSIGNED_INT, 0, 1);
				}
				else
				{
					glDrawElementsInstanced(GL_TRIANGLES, uv.get_indices().size(), GL_UNSIGNED_INT, 0, 1);
				}
			}


			// draw AABB
			if (m_draw_aabb) {

				m_shader_aabb.use();
				glBindVertexArray(m_VAO_BOX_AABB);

				glUniformMatrix4fv(glGetUniformLocation(m_shader_aabb.get_id(), "view"), 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(glGetUniformLocation(m_shader_aabb.get_id(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
				glUniformMatrix4fv(glGetUniformLocation(m_shader_aabb.get_id(), "model"), 1, GL_FALSE, glm::value_ptr(model));

				glUniform3f(glGetUniformLocation(m_shader_aabb.get_id(), "color"), aabb_color[0], aabb_color[1], aabb_color[2]);

				glDrawElementsInstanced(GL_LINES, 24, GL_UNSIGNED_INT, 0, m_loader.get_gaussians().size());
			}

			// draw BVH
			if (m_draw_bvh) {

				m_shader_aabb.use();
				glBindVertexArray(m_VAO_BOX_BVH);

				glUniformMatrix4fv(glGetUniformLocation(m_shader_aabb.get_id(), "view"), 1, GL_FALSE, glm::value_ptr(view));
				glUniformMatrix4fv(glGetUniformLocation(m_shader_aabb.get_id(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
				glUniformMatrix4fv(glGetUniformLocation(m_shader_aabb.get_id(), "model"), 1, GL_FALSE, glm::value_ptr(model));

				glUniform3f(glGetUniformLocation(m_shader_aabb.get_id(), "color"), bvh_color[0], bvh_color[1], bvh_color[2]);
				
				glDrawElementsInstanced(GL_LINES, 24, GL_UNSIGNED_INT, 0, m_bvh.size());
			}

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			
			ImGui::Begin("Settings");

			if (ImGui::Checkbox("Draw AABB", &m_draw_aabb))
				init_aabb();
			ImGui::SameLine();
			ImGui::ColorEdit3("##aabb_color", aabb_color.data(), ImGuiColorEditFlags_NoInputs);


			if (ImGui::Checkbox("Draw BVH", &m_draw_bvh))
				init_bvh();
			ImGui::SameLine();
			ImGui::ColorEdit3("##bvh_color", bvh_color.data(), ImGuiColorEditFlags_NoInputs);

			
			ImGui::Text("Ambient Color:");
			ImGui::SameLine();
			ImGui::ColorEdit3("##ambient_color", ambient_color.data(), ImGuiColorEditFlags_NoInputs);
			ImGui::SameLine();
			ImGui::SliderFloat("Ambient Strength", &ambient_strength, 0.0f, 1.0f);

			ImGui::Text("Color Light Source:");
			ImGui::SameLine();
			ImGui::ColorEdit3("##light_source_color", light_color.data(), ImGuiColorEditFlags_NoInputs);

			ImGui::Checkbox("Draw points:", &draw_points);


			if (ImGui::SliderFloat("Gaussian Q", &m_gaussian_Q, 0.1f, 10.0f))
			{
				for (std::size_t idx = 0; idx < m_loader.get_gaussians().size(); ++idx)
				{
					m_loader.get_gaussians()[idx].Q = std::sqrt(m_gaussian_Q);
				}

				glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_Gaussians);
				glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_loader.get_gaussians().size() * sizeof(Ellipsoid), m_loader.get_gaussians().data());
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
			}
			ImGui::End();

			ImGui::Text("FPS: %lf", fps);

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			glfwSwapBuffers(m_window);
			
			glFinish();
			end_time = std::chrono::high_resolution_clock::now();

			fps = 1.0f / std::chrono::duration<float>(end_time - start_time).count();
			
			glfwPollEvents();


		}
	}


private:

	bool init_imgui()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui_ImplGlfw_InitForOpenGL(m_window, true);
		ImGui_ImplOpenGL3_Init("#version 130");

		return true;
	}

	bool init_opengl()
	{
		glfwInit();

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		std::cout << m_width << "\t" << m_height << std::endl;

		m_window = glfwCreateWindow(m_width, m_height, "Gaussians Viewer", nullptr, nullptr);

		std::cout << m_width << "\t" << m_height << std::endl;

		if (m_window == nullptr)
		{
			std::cerr << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return false;
		}

		glfwMakeContextCurrent(m_window);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cerr << "Failed to initialize GLAD" << std::endl;
			return false;
		}

		// glfwSwapInterval(0); // disable VSync

		set_callbacks();

		return true;
	}

	void setup_uv()
	{

	}

	void set_callbacks()
	{
		glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
		glfwSetCursorPosCallback(m_window, mouse_callback);
		glfwSetScrollCallback(m_window, scroll_callback);
		glfwSetMouseButtonCallback(m_window, mouse_button_callback);
	}

	static void init_camera()
	{
		m_height = 720;
		m_width = 1280;
		m_camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));
		m_last_coords = { m_width / 2.0f, m_height / 2.0f };
		m_first_mouse = true;
		m_delta_time = 0.0f;
		m_last_frame = 0.0f;
	}

	void load_shaders()
	{
		m_shader_gaussians.create();
		m_shader_gaussians.load_shaders(std::make_pair("C:\\dev\\Gaussian_Splatting\\3DGS\\Source\\gaussians_viewer\\shaders\\vertex_shader_gaussians.vert", GL_VERTEX_SHADER), std::make_pair("C:\\dev\\Gaussian_Splatting\\3DGS\\Source\\gaussians_viewer\\shaders\\fragment_shader_gaussians.frag", GL_FRAGMENT_SHADER));
		m_shader_gaussians.link();

		m_shader_aabb.create();
		m_shader_aabb.load_shaders(std::make_pair("C:\\dev\\Gaussian_Splatting\\3DGS\\Source\\gaussians_viewer\\shaders\\vertex_shader_aabb.vert", GL_VERTEX_SHADER), std::make_pair("C:\\dev\\Gaussian_Splatting\\3DGS\\Source\\gaussians_viewer\\shaders\\fragment_shader_aabb.frag", GL_FRAGMENT_SHADER));
		m_shader_aabb.link();

		m_shader_light_source.create();
		m_shader_light_source.load_shaders(std::make_pair("C:\\dev\\Gaussian_Splatting\\3DGS\\Source\\gaussians_viewer\\shaders\\vertex_shader_light_source.vert", GL_VERTEX_SHADER), std::make_pair("C:\\dev\\Gaussian_Splatting\\3DGS\\Source\\gaussians_viewer\\shaders\\fragment_shader_light_source.frag", GL_FRAGMENT_SHADER));
		m_shader_light_source.link();
	}

	void init_gaussians()
	{
		// create scene
		// m_loader.create_random_scene(100);
		// m_loader.create_scene_from_file("C:\\dev\\Gaussian_Splatting\\Splatshop\\splatmodels\\splats\\point_cloud.ply");
		m_loader.create_scene_from_file("C:\\dev\\Gaussian_Splatting\\3DGS\\Source\\gaussians_viewer\\test_data\\test_1_iteration\\point_cloud.ply"); // set up my own directory
		// m_loader.create_scene_from_file("C:\\dev\\Gaussian_Splatting\\gaussian-splatting\\output\\827a55cb-5\\point_cloud\\iteration_7000\\point_cloud.ply");

		std::cout << m_loader.get_gaussians().size() << " gaussians loaded." << std::endl;

		glGenVertexArrays(1, &m_VAO_Gaussians);

		glBindVertexArray(m_VAO_Gaussians);

		glGenBuffers(1, &m_SSBO_Gaussians);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_Gaussians);
		glBufferData(GL_SHADER_STORAGE_BUFFER, m_loader.get_gaussians().size() * sizeof(Ellipsoid), m_loader.get_gaussians().data(), GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_SSBO_Gaussians);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		glBindVertexArray(0);

	}

	void init_bvh()
	{
		if (m_draw_bvh_initialize == false)
		{
			// Setting for BOX
			glGenVertexArrays(1, &m_VAO_BOX_BVH);
			
			glBindVertexArray(m_VAO_BOX_BVH);
			
			glGenBuffers(1, &m_VBO_BOX_BVH);
			glBindBuffer(GL_ARRAY_BUFFER, m_VBO_BOX_BVH);
			glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), AABB::cube_vertices, GL_STATIC_DRAW);
			
			glGenBuffers(1, &m_EBO_BOX_BVH);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_BOX_BVH);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24 * sizeof(unsigned int), AABB::cube_indices, GL_STATIC_DRAW);
			
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			
			// Setting for BVH
			glGenBuffers(1, &m_VBO_BVH);
			glBindBuffer(GL_ARRAY_BUFFER, m_VBO_BVH);
			glBufferData(GL_ARRAY_BUFFER, m_bvh.size() * sizeof(AABB), m_bvh.data(), GL_STATIC_DRAW);

			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AABB), (void*)0);
			glEnableVertexAttribArray(1);

			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(AABB), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(2);

			glVertexAttribDivisor(1, 1);
			glVertexAttribDivisor(2, 1);

			glBindVertexArray(0);

			m_draw_bvh_initialize = true;
		}
	}

	void init_aabb()
	{
		if (m_draw_aabb_initialize == false)
		{
			std::vector<AABB> aabb;
			generate_AABB_from_gaussians(m_loader.get_gaussians(), aabb);

			// Setting for BOX
			glGenVertexArrays(1, &m_VAO_BOX_AABB);

			glBindVertexArray(m_VAO_BOX_AABB);

			glGenBuffers(1, &m_VBO_BOX_AABB);
			glBindBuffer(GL_ARRAY_BUFFER, m_VBO_BOX_AABB);
			glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), AABB::cube_vertices, GL_STATIC_DRAW);

			glGenBuffers(1, &m_EBO_BOX_AABB);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_BOX_AABB);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24 * sizeof(unsigned int), AABB::cube_indices, GL_STATIC_DRAW);

			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);

			glGenBuffers(1, &m_VBO_AABB);
			glBindBuffer(GL_ARRAY_BUFFER, m_VBO_AABB);
			glBufferData(GL_ARRAY_BUFFER, aabb.size() * sizeof(AABB), aabb.data(), GL_STATIC_DRAW);

			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AABB), (void*)0);
			glEnableVertexAttribArray(1);

			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(AABB), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(2);

			glVertexAttribDivisor(1, 1);
			glVertexAttribDivisor(2, 1);

			glBindVertexArray(0);

			m_draw_aabb_initialize = true;
		}
	}

	// callbacks
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		ImGuiIO& io = ImGui::GetIO();

		if (button == GLFW_MOUSE_BUTTON_LEFT)
		{
			if (action == GLFW_PRESS && !io.WantCaptureMouse && !m_mouse_press)
			{
				m_mouse_press = true;
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			else if (action == GLFW_RELEASE && m_mouse_press)
			{
				m_mouse_press = false;
				m_first_mouse = true;
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
		}
	}

	static void process_input(GLFWwindow* window)
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			m_camera.process_keyboard(FORWARD, m_delta_time);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			m_camera.process_keyboard(BACKWARD, m_delta_time);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			m_camera.process_keyboard(LEFT, m_delta_time);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			m_camera.process_keyboard(RIGHT, m_delta_time);
	}


	static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
	{
		if (m_mouse_press == false || ImGui::GetIO().WantCaptureMouse)
			return;


		float xpos = static_cast<float>(xposIn);
		float ypos = static_cast<float>(yposIn);

		if (m_first_mouse)
		{
			m_last_coords = { xpos, ypos };
			m_first_mouse = false;
		}

		float xoffset = xpos - m_last_coords.first;
		float yoffset = m_last_coords.second - ypos; // reversed since y-coordinates go from bottom to top

		m_last_coords = { xpos, ypos };

		m_camera.process_mouse_movement(xoffset, yoffset);
	}

	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		m_camera.process_mouse_scroll(static_cast<float>(yoffset));
	}

};


Camera App::m_camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));

float App::m_delta_time = 0.0f;
float App::m_last_frame = 0.0f;

unsigned int App::m_width = 1280;
unsigned int App::m_height = 720;

std::pair<float, float> App::m_last_coords = { (float)App::m_width / 2.0f, (float)App::m_height / 2.0f };
bool App::m_first_mouse = true;
bool App::m_mouse_press = false;

#endif // !APP_H
