#version 430 core

uniform vec3 diffuse_color;

out vec4 FragColor;

void main()
{
	FragColor = vec4(diffuse_color, 1.0f);
}