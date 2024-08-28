#ifndef RENDERER_H
#define RENDERER_H

#include <Shader.hpp>
#include <Arena.hpp>
#include <cstdint>

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
	void render(float tex_offset, std::pair<uint16_t, uint16_t> pos, std::pair<uint16_t, uint16_t> size);
};


#endif
