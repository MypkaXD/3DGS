#version 430 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 center;
layout (location = 2) in vec3 size;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

out float v_opacity;

void main()
{
    v_opacity = 1.0f;
    vec3 worldPos = center + pos * size;
    gl_Position = projection * view * model * vec4(worldPos,1.0);
}