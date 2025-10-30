#version 330 core

layout (location = 0) in vec4 px_pos;
layout (location = 1) in float tex_offset;
layout (location = 2) in mat4 transform;

uniform mat4 projection;

out vec2 tex_coords;
out float offset;

void main()
{
	tex_coords = px_pos.zw;
	offset = tex_offset;
	gl_Position = projection * transform * vec4(px_pos.xy, 0.0, 1.0);
}
