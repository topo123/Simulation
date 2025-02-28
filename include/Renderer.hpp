#ifndef RENDERER_H
#define RENDERER_H

#include <Shader.hpp>
#include <Arena.hpp>
#include <Material.hpp>

class Renderer
{
	unsigned int scr_width = 800, scr_height = 600;
	unsigned int FBO;
	unsigned int RBO;
	unsigned int color_buffer;
	unsigned int quad;
	unsigned int shader_program;
	unsigned int texture;
	Shader* shader;
	Arena* arena;

	void compile_shaders();

public:
	void draw_rect(vector2& upper, vector2& lower, float offset);
	void initRenderData();
	void render(float tex_offset, vector2* pos, vector2* size);
	~Renderer();
};


#endif
