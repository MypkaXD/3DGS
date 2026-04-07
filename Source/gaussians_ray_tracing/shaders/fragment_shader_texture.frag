#version 430 core

uniform sampler2D compute_texture;

in vec2 tex_coord;

out vec4 FragColor;

void main()
{
	FragColor = texture(compute_texture, tex_coord);
	// FragColor = vec4(tex_coord, 0.0, 1.0);
}