#version 430 core

layout (location = 0) in vec2 pos_uv; // Spherical coordinates (theta, phi)

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

uniform vec3 light_pos;

void main()
{

    // Convert spherical coordinates
    vec3 sphere_point = vec3(sin(pos_uv.y) * cos(pos_uv.x), sin(pos_uv.y) * sin(pos_uv.x), cos(pos_uv.y)); // x ~ phi = [0, 2PI], y ~ theta = [0, PI]

    gl_Position = projection * view * model * vec4(sphere_point + light_pos, 1.0);
}