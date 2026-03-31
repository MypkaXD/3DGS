#version 430 core

in vec3 out_color;
out vec4 FragColor;

void main()
{
	vec3 light_source = vec3(0.0f, 1.0f, 0.0f);
	FragColor = vec4(out_color * light_source, 1.0f);
}