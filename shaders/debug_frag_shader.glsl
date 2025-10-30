
#version 330 core

out vec4 FragColor;

in vec2 tex_coords;

uniform sampler1D tex;
uniform float tex_offset;

void main()
{
	FragColor = texture(tex, tex_offset);
}
