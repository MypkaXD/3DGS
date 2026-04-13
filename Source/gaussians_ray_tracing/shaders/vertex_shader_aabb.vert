#version 430 core

layout (location = 0) in vec3 center;
layout (location = 1) in vec3 size;
layout (location = 2) in vec3 pos;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

void main()
{
    vec3 worldPos = center + pos * size;
    gl_Position = projection * view * model * vec4(worldPos,1.0);
}