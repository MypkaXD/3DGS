#include <iostream>
#include <array>

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

#include <Camera.h>
#include <Shader.h>

unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

int main()
{
	
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SimpleRayTracing", nullptr, nullptr);

	if (window == nullptr)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 440");

	Shader compute_shader;
	compute_shader.create();
	compute_shader.load_shaders(std::make_pair("C:\\dev\\Gaussian_Splatting\\3DGS\\Source\\simple_ray_tracing\\shaders\\compute_shader.comp", GL_COMPUTE_SHADER));
	compute_shader.link();

	Shader shader;
	shader.create();
	shader.load_shaders(std::make_pair("C:\\dev\\Gaussian_Splatting\\3DGS\\Source\\simple_ray_tracing\\shaders\\vertex_shader.vert", GL_VERTEX_SHADER), std::make_pair("C:\\dev\\Gaussian_Splatting\\3DGS\\Source\\simple_ray_tracing\\shaders\\fragment_shader.frag", GL_FRAGMENT_SHADER));
	shader.link();

	unsigned int compute_texure;
	glGenTextures(1, &compute_texure);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, compute_texure);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glBindImageTexture(0, compute_texure, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

	float vertices[]  = {
		-1.0f, -1.0f , 0.0f, 0.0f, 0.0f,
		-1.0f,  1.0f , 0.0f, 0.0f, 1.0f,
		 1.0f,  1.0f , 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f , 0.0f, 1.0f, 0.0f,
	};

	unsigned int indices[] = {
		0, 2, 1,
		0, 3, 2
	};

	unsigned int VAO, VBO, EBO;

	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	std::array<float, 3> light_source = { 10.0f, 10.0f, -5.0f };
	std::array<float, 3> sphere_center = { 0.2f, 0.2f, 10.0f };

	while (!glfwWindowShouldClose(window))
	{

		float current_time = static_cast<float>(glfwGetTime());

		light_source[0] = 10.0f * cos(current_time);
		light_source[1] = 10.0f * sin(current_time);

		sphere_center[0] = 0.5f * cos(current_time);
		sphere_center[1] = 0.5f * sin(current_time);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		compute_shader.use();
		glUniform3f(glGetUniformLocation(compute_shader.get_id(), "light_source"), light_source[0], light_source[1], light_source[2]);
		glUniform3f(glGetUniformLocation(compute_shader.get_id(), "sphere_center"), sphere_center[0], sphere_center[1], sphere_center[2]);
		glDispatchCompute(SCR_WIDTH, SCR_HEIGHT, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		shader.use();
		glBindVertexArray(VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, compute_texure);
		glUniform1i(glGetUniformLocation(shader.get_id(), "compute_texture"), 0);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Debug");
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();

	return 0;
}