#version 330 core

out vec4 FragColor;

in vec2 tex_coords;

uniform float offset;
uniform sampler1D tex;

void main()
{
	FragColor = texture(tex, offset);
}
