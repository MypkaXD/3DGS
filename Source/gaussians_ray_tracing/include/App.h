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

	Shader m_shader_compute_gaussian;
	Shader m_shader_texture_draw;

	// Camera settings
	static Camera m_camera;
	static std::pair<float, float> m_last_coords;
	static bool m_first_mouse;
	static bool m_mouse_press;

	// Timing
	static float m_delta_time;	// time between current frame and last frame
	static float m_last_frame;

	float m_gaussian_Q = 1.0f;


	unsigned int m_VAO_draw_texture;
	unsigned int m_VBO_draw_texture;
	unsigned int m_EBO_draw_texture;

	unsigned int m_texture_draw;

	unsigned int m_VAO_gaussians;
	unsigned int m_SSBO_gaussians;
	unsigned int m_SSBO_bvh_nodes;
	unsigned int m_SSBO_bvh_ids;

	std::vector<AABB> m_bvh;
	BVHGPU m_bvh_gpu;

public:

	App()
	{
	}

	~App()
	{

		glDeleteVertexArrays(1, &m_VAO_draw_texture);
		glDeleteBuffers(1, &m_VBO_draw_texture);
		glDeleteBuffers(1, &m_EBO_draw_texture);

		glDeleteTextures(1, &m_texture_draw);

		glDeleteVertexArrays(1, &m_VAO_gaussians);
		glDeleteBuffers(1, &m_SSBO_gaussians);
		glDeleteBuffers(1, &m_SSBO_bvh_nodes);
		glDeleteBuffers(1, &m_SSBO_bvh_ids);

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

		std::vector<AABB> aabb;
		generate_AABB_from_gaussians(m_loader.get_gaussians(), aabb);

		create_bvh(aabb, m_bvh, m_bvh_gpu);

		return true;
	}

	void main_loop()
	{

		auto start_time = std::chrono::high_resolution_clock::now();
		auto end_time = std::chrono::high_resolution_clock::now();
		auto fps = 0.0f;

		float vertices_texture[] = {
			-1.0f, -1.0f , 0.0f, 0.0f, 0.0f,
			-1.0f,  1.0f , 0.0f, 0.0f, 1.0f,
			 1.0f,  1.0f , 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f , 0.0f, 1.0f, 0.0f,
		};

		unsigned int indices_texture[] = {
			0, 2, 1,
			0, 3, 2
		};

		glGenVertexArrays(1, &m_VAO_draw_texture);
		glBindVertexArray(m_VAO_draw_texture);

		glGenBuffers(1, &m_VBO_draw_texture);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO_draw_texture);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_texture), vertices_texture, GL_STATIC_DRAW);

		glGenBuffers(1, &m_EBO_draw_texture);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_draw_texture);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices_texture), indices_texture, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glBindVertexArray(0);

		glGenTextures(1, &m_texture_draw);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texture_draw);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
		glBindImageTexture(0, m_texture_draw, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

		glGenVertexArrays(1, &m_VAO_gaussians);
		glBindVertexArray(m_VAO_gaussians);

		glGenBuffers(1, &m_SSBO_gaussians);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_gaussians);
		glBufferData(GL_SHADER_STORAGE_BUFFER, m_loader.get_gaussians().size() * sizeof(Ellipsoid), m_loader.get_gaussians().data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_SSBO_gaussians);
			
		glGenBuffers(1, &m_SSBO_bvh_nodes);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_bvh_nodes);
		glBufferData(GL_SHADER_STORAGE_BUFFER, m_bvh_gpu.nodes.size() * sizeof(BVHNodeGPU), m_bvh_gpu.nodes.data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_SSBO_bvh_nodes);

		glGenBuffers(1, &m_SSBO_bvh_ids);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_bvh_ids);
		glBufferData(GL_SHADER_STORAGE_BUFFER, m_bvh_gpu.prim_ids.size() * sizeof(size_t), m_bvh_gpu.prim_ids.data(), GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_SSBO_bvh_ids);
		glBindVertexArray(0);

		GLint camera_pos_loc = glGetUniformLocation(m_shader_compute_gaussian.get_id(), "camera_pos");
		GLint camera_front_loc = glGetUniformLocation(m_shader_compute_gaussian.get_id(), "camera_front");
		GLint camera_right_loc = glGetUniformLocation(m_shader_compute_gaussian.get_id(), "camera_right");
		GLint camera_up_loc = glGetUniformLocation(m_shader_compute_gaussian.get_id(), "camera_up");
		GLint fov_loc = glGetUniformLocation(m_shader_compute_gaussian.get_id(), "fov");
		GLint aspect_loc = glGetUniformLocation(m_shader_compute_gaussian.get_id(), "aspect");

		while (!glfwWindowShouldClose(m_window))
		{

			start_time = std::chrono::high_resolution_clock::now();

			float currentFrame = static_cast<float>(glfwGetTime());
			m_delta_time = currentFrame - m_last_frame;
			m_last_frame = currentFrame;

			process_input(m_window);

			//glm::mat4 view = m_camera.get_view_matrix(); // camera matrix
			//glm::mat4 projection = glm::perspective(glm::radians(m_camera.Zoom), (float)m_width / (float)m_height, 0.1f, 1000.0f); // projection matrix
			//glm::mat4 model = glm::mat4(1.0f); // model matrix

			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			m_shader_compute_gaussian.use();
			glBindVertexArray(m_VAO_gaussians);
			glUniform3f(camera_pos_loc, m_camera.Position.x, m_camera.Position.y, m_camera.Position.z);
			glUniform3f(camera_front_loc, m_camera.Front.x, m_camera.Front.y, m_camera.Front.z);
			glUniform3f(camera_right_loc, m_camera.Right.x, m_camera.Right.y, m_camera.Right.z);
			glUniform3f(camera_up_loc, m_camera.Up.x, m_camera.Up.y, m_camera.Up.z);
			glUniform1f(fov_loc, m_camera.Zoom);
			float aspect = (float)m_width / (float)m_height;
			glUniform1f(aspect_loc, aspect);
			glDispatchCompute((m_width + 7) / 8, (m_height + 3) / 4, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			m_shader_texture_draw.use();
			glBindVertexArray(m_VAO_draw_texture);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_texture_draw);
			glUniform1i(glGetUniformLocation(m_shader_texture_draw.get_id(), "compute_texture"), 0);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			
			ImGui::Begin("Settings");

			ImGui::Text("FPS: %lf", fps);

			ImGui::End();

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

		glfwSetWindowUserPointer(m_window, this);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cerr << "Failed to initialize GLAD" << std::endl;
			return false;
		}

		// glfwSwapInterval(0); // disable VSync

		set_callbacks();

		return true;
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
		m_shader_texture_draw.create();
		m_shader_texture_draw.load_shaders(
			std::make_pair("C:\\dev\\Gaussian_Splatting\\3DGS\\Source\\gaussians_ray_tracing\\shaders\\vertex_shader_texture.vert", GL_VERTEX_SHADER),
			std::make_pair("C:\\dev\\Gaussian_Splatting\\3DGS\\Source\\gaussians_ray_tracing\\shaders\\fragment_shader_texture.frag", GL_FRAGMENT_SHADER)
		);
		m_shader_texture_draw.link();

		m_shader_compute_gaussian.create();
		m_shader_compute_gaussian.load_shaders(
			std::make_pair("C:\\dev\\Gaussian_Splatting\\3DGS\\Source\\gaussians_ray_tracing\\shaders\\compute_shader.comp", GL_COMPUTE_SHADER)
		);
		m_shader_compute_gaussian.link();
	}

	void init_gaussians()
	{
		// create scene
		// m_loader.create_random_scene(1000);
		// m_loader.create_scene_from_file("C:\\dev\\Gaussian_Splatting\\Splatshop\\splatmodels\\splats\\point_cloud.ply");
		m_loader.create_scene_from_file("C:\\dev\\Gaussian_Splatting\\3DGS\\Source\\gaussians_viewer\\test_data\\test_1_iteration\\point_cloud.ply"); // set up my own directory
		// m_loader.create_scene_from_file("C:\\dev\\Gaussian_Splatting\\gaussian-splatting\\output\\827a55cb-5\\point_cloud\\iteration_7000\\point_cloud.ply");

		std::cout << m_loader.get_gaussians().size() << " gaussians loaded." << std::endl;

	}

	// callbacks
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		auto instance = static_cast<App*>(glfwGetWindowUserPointer(window));

		std::cout << instance << std::endl;

		if (instance)
		{
			glBindTexture(GL_TEXTURE_2D, instance->m_texture_draw);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
			glBindImageTexture(0, instance->m_texture_draw, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		}

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
