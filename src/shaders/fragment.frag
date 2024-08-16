#version 330 core

out vec4 FragColor;

in vec2 tex_coords;

uniform float offset;
uniform sampler2D tex;

void main()
{
	vec2 adjust = vec2(tex_coords.x/2.0 + offset, tex_coords.y);
	FragColor = texture(tex, adjust);
}
