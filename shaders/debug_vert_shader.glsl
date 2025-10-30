#version 330 core

layout (location = 0) in vec2 line_point;
layout (location = 1) in mat4 debug_transform;

uniform mat4 projection;

void main()
{
	gl_Position = projection * debug_transform * vec4(line_point.xy, 0, 1.0f);
}
