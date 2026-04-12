#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Shader
{
private:

	unsigned int m_shader_id;

public:

	Shader() {}

	~Shader()
	{
		glDeleteProgram(m_shader_id);
	}

	void create()
	{
		m_shader_id = glCreateProgram();
	}

	template<typename... Args>
	void load_shaders(Args... args)
	{
		(load_shader(args), ...);
	}

	void load_shader(std::pair<const std::string, unsigned int> data)
	{
		std::ifstream file(data.first.c_str());
		
		if (file.is_open())
		{
			std::stringstream shader_stream;
			shader_stream << file.rdbuf();
			file.close();

			std::string shader_code = shader_stream.str();
			const char* shader_code_cstr = shader_code.c_str();

			unsigned int shader = glCreateShader(data.second);
			glShaderSource(shader, 1, &shader_code_cstr, NULL);
			glCompileShader(shader);
			glAttachShader(m_shader_id, shader);
			glDeleteShader(shader);
		}
		else
		{
			std::cout << "Error: Failed to load shader " << data.first << std::endl;
			return;
		}
	}

	void link()
	{
		glLinkProgram(m_shader_id);
	}

	void use() {
		glUseProgram(m_shader_id);
	}

	unsigned int get_id() const {
		return m_shader_id;
	}

	void set_float(const unsigned int location, float value) const
	{
		glUniform1f(location, value);
	}

	void set_float3(const unsigned int location, float x, float y, float z) const
	{
		glUniform3f(location, x, y, z);
	}

	void set_vec3(const unsigned int location, const glm::vec3& value) const
	{
		glUniform3f(location, value.x, value.y, value.z);
	}

	void set_int(const unsigned int location, int value) const
	{
		glUniform1i(location, value);
	}

};


#endif // !SHADER_H
