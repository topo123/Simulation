#version 330 core

layout (location = 0) in vec2 screen_coords;
layout (location = 1) in vec2 tex_coords;

out vec2 TexCoord;

void main()
{
	gl_Position = vec4(screen_coords.xy, 0.0, 1.0);
	TexCoord = tex_coords;
}
