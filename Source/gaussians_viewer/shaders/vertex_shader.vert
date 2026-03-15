#version 330 core

layout (location = 0) in vec2 pos_uv;
layout (location = 1) in vec3 mu;
layout (location = 2) in vec3 sigma;
layout (location = 3) in float q;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

void main()
{
	vec3 pos = vec3(
		mu.x + sigma.x * q * sin(pos_uv.y) * cos(pos_uv.x),
		mu.y + sigma.y * q * sin(pos_uv.y) * sin(pos_uv.x),
		mu.z + sigma.z * q * cos(pos_uv.y)
	);
	gl_Position = projection * view * model * vec4(pos, 1.0);
}