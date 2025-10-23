#ifndef RENDERER_H
#define RENDERER_H

#include <Shader.hpp>
#include <Arena.hpp>
#include <Material.hpp>

class Renderer
{
	unsigned int scr_width = 800, scr_height = 600;

	unsigned int FBO;

	unsigned int write_texture_quad;
	unsigned int screen_quad;

	unsigned int writer_shader_program;
	unsigned int screen_shader_program;

	unsigned int material_texture;
	unsigned int write_texture;

	Arena* arena;

	void compile_shaders(unsigned int world_width, unsigned int world_height);

public:
	void draw_texture_rect(vector2& upper, vector2& lower, float offset);
	void initRenderData(unsigned int world_width, unsigned int world_height);
	void texture_render(float tex_offset, vector2* pos, vector2* size);
	void screen_render();
	~Renderer();
};


#endif
