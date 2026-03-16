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

#include <Shader.h>
#include <Camera.h>
#include <Ellipsoid.h>
#include <AABB.h>

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


void generate_gaussians(std::vector<Ellipsoid>& data, std::size_t n)
{
	std::random_device rd;
	std::mt19937 gen(rd());

	std::uniform_int_distribution<int> dis_mu(-100, 100);
	std::uniform_real_distribution<float> dis_sigma(1, 2);
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

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);

	glEnable(GL_DEPTH_TEST);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 130");

	std::vector<Ellipsoid> gaussians;
	generate_gaussians(gaussians, 1000);

	std::vector<AABB> aabb;
	generate_AABB(gaussians, aabb);

	UV uv(10, 10, 0.0f, 2.0f * M_PI, 0.0f, M_PI);
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
	bool draw_AABB = false;

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