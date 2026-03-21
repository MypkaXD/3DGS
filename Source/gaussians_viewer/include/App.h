#ifndef APP_H
#define APP_H

#include <iostream>


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

	// Buffers of gaussians
	unsigned int m_VAO_Gaussians;
	unsigned int m_VBO_Gaussians;

	// Buffers for gaussians Solid
	unsigned int m_VBO_UV;
	unsigned int m_EBO_UV;

	// Shaders
	Shader m_shader_gaussians;
	Shader m_shader_aabb;

	// Camera settings
	static Camera m_camera;
	static std::pair<float, float> m_last_coords;
	static bool m_first_mouse;

	// Timing
	static float m_delta_time;	// time between current frame and last frame
	static float m_last_frame;

	UV uv;
public:

	App()
	{
	}

	~App()
	{
		if (m_sucessfully_initialized)
		{
			glDeleteVertexArrays(1, &m_VAO_Gaussians);

			glDeleteBuffers(1, &m_VBO_Gaussians);
			glDeleteBuffers(1, &m_VBO_UV);
			glDeleteBuffers(1, &m_EBO_UV);

			m_shader_gaussians.~Shader();
			m_shader_aabb.~Shader();
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

		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		glEnable(GL_DEPTH_TEST);

		load_shaders();

		init_gaussians();

		uv = UV(20, 20, 0.0f, 2.0f * M_PI, 0.0f, M_PI);
		uv.generate();

		glBindVertexArray(m_VAO_Gaussians);

		glGenBuffers(1, &m_VBO_UV);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO_UV);
		glBufferData(GL_ARRAY_BUFFER,
			uv.get_vertices().size() * sizeof(float),
			uv.get_vertices().data(),
			GL_STATIC_DRAW);

		glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(5);

		glGenBuffers(1, &m_EBO_UV);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_UV);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			uv.get_indices().size() * sizeof(unsigned int),
			uv.get_indices().data(),
			GL_STATIC_DRAW);

		glBindVertexArray(0);

		bool draw_ellipsoid_by_lines = false;
		bool draw_AABB = true;

		m_sucessfully_initialized = true;

		return true;
	}

	void main_loop()
	{
		while (!glfwWindowShouldClose(m_window))
		{
			float currentFrame = static_cast<float>(glfwGetTime());
			m_delta_time = currentFrame - m_last_frame;
			m_last_frame = currentFrame;

			process_input(m_window);

			glm::mat4 view = m_camera.get_view_matrix(); // camera matrix
			glm::mat4 projection = glm::perspective(glm::radians(m_camera.Zoom), (float)m_width / (float)m_height, 0.1f, 1000.0f); // projection matrix
			glm::mat4 model = glm::mat4(1.0f); // model matrix

			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			m_shader_gaussians.use();

			glUniformMatrix4fv(glGetUniformLocation(m_shader_gaussians.get_id(), "view"), 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(m_shader_gaussians.get_id(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			glUniformMatrix4fv(glGetUniformLocation(m_shader_gaussians.get_id(), "model"), 1, GL_FALSE, glm::value_ptr(model));

			// draw ellipsoids
			{
				glBindVertexArray(m_VAO_Gaussians);
				glDrawElementsInstanced(GL_TRIANGLES,
					static_cast<GLsizei>(uv.get_indices().size()),
					GL_UNSIGNED_INT,
					0,
					static_cast<GLsizei>(m_loader.get_gaussians().size()));
			}

			// drwa AABB
			//{
			//	if (draw_AABB) {

			//		shader_aabb.use();
			//		glBindVertexArray(VAO_AABB);
			//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_AABB);

			//		glUniformMatrix4fv(glGetUniformLocation(shader_aabb.get_id(), "view"), 1, GL_FALSE, glm::value_ptr(view));
			//		glUniformMatrix4fv(glGetUniformLocation(shader_aabb.get_id(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			//		glUniformMatrix4fv(glGetUniformLocation(shader_aabb.get_id(), "model"), 1, GL_FALSE, glm::value_ptr(model));

			//		glDrawElementsInstanced(
			//			GL_LINES,
			//			24,
			//			GL_UNSIGNED_INT,
			//			0,
			//			aabb.size()
			//		);
			//	}
			//}

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
			ImGui::Begin("Settings");
			//ImGui::Checkbox("Draw ellipsoids by lines", &draw_ellipsoid_by_lines);
			//ImGui::Checkbox("Draw AABB", &draw_AABB);
			ImGui::End();
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			glfwSwapBuffers(m_window);
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

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
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

		set_callbacks();

		return true;
	}

	void set_callbacks()
	{
		glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
		glfwSetCursorPosCallback(m_window, mouse_callback);
		glfwSetScrollCallback(m_window, scroll_callback);
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
		m_shader_gaussians.load_shaders(std::make_pair("C:\\dev\\Gaussian_Splatting\\3DGS\\Source\\gaussians_viewer\\shaders\\vertex_shader_gaussians.vert", GL_VERTEX_SHADER), std::make_pair("C:\\dev\\Gaussian_Splatting\\3DGS\\Source\\gaussians_viewer\\shaders\\fragment_shader.frag", GL_FRAGMENT_SHADER));
		m_shader_gaussians.link();

		m_shader_aabb.create();
		m_shader_aabb.load_shaders(std::make_pair("C:\\dev\\Gaussian_Splatting\\3DGS\\Source\\gaussians_viewer\\shaders\\vertex_shader_aabb.vert", GL_VERTEX_SHADER), std::make_pair("C:\\dev\\Gaussian_Splatting\\3DGS\\Source\\gaussians_viewer\\shaders\\fragment_shader.frag", GL_FRAGMENT_SHADER));
		m_shader_aabb.link();
	}

	void init_gaussians()
	{
		// create scene
		m_loader.create_random_scene(10000);
		// m_loader.create_scene_from_file("C:\\dev\\Gaussian_Splatting\\Splatshop\\splatmodels\\splats\\point_cloud.ply");

		std::cout << m_loader.get_gaussians().size() << " gaussians loaded." << std::endl;

		// init buffer for gaussians
		glGenVertexArrays(1, &m_VAO_Gaussians);
		glGenBuffers(1, &m_VBO_Gaussians);

		glBindVertexArray(m_VAO_Gaussians);

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO_Gaussians);
		glBufferData(GL_ARRAY_BUFFER, m_loader.get_gaussians().size() * sizeof(Ellipsoid), m_loader.get_gaussians().data(), GL_STATIC_DRAW);

		// mu
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Ellipsoid), (void*)0);
		glEnableVertexAttribArray(0);

		// Sigma
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Ellipsoid), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		// Quaternion
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Ellipsoid), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		// Opacity
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Ellipsoid), (void*)(10 * sizeof(float)));
		glEnableVertexAttribArray(3);

		// Q
		glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Ellipsoid), (void*)(11 * sizeof(float)));
		glEnableVertexAttribArray(4);

		glVertexAttribDivisor(0, 1);
		glVertexAttribDivisor(1, 1);
		glVertexAttribDivisor(2, 1);
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);

		glBindVertexArray(0);

	}

	// callbacks
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
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

#endif // !APP_H
