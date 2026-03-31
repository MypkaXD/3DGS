#version 430 core

uniform float ambient_strength;
uniform vec3 ambient_color;

in vec3 out_color;

out vec4 FragColor;

void main()
{
	vec3 ambient = ambient_strength * ambient_color;

	FragColor = vec4(out_color + ambient, 1.0f);
}