#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D write_texture;

void main()
{
	FragColor = texture(write_texture, TexCoord);
}
