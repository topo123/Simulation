#ifndef RENDERER_H
#define RENDERER_H

#include <Shader.hpp>
#include <Arena.hpp>
#include <Material.hpp>

class Renderer
{
	unsigned int quad;
	unsigned int shader_program;
	unsigned int texture;
	Shader* shader;
	Arena* arena;

	void compile_shaders();

public:
	void initRenderData();
	void render(float tex_offset, vector2* pos, vector2* size);
};


#endif
