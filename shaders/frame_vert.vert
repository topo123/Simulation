#version 330 core
layout (location = 0) in vec4 px_pos;

uniform mat4 model;
uniform mat4 projection;

out vec2 tex_coords;

void main()
{
	tex_coords = px_pos.zw;
	gl_Position = projection * model * vec4(px_pos.xy, 0.0, 1.0);
}
