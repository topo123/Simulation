#ifndef RENDERER_H
#define RENDERER_H

#include <Shader.hpp>
#include <Arena.hpp>
#include <Material.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Renderer
{
	unsigned int scr_width = 800, scr_height = 600;

	unsigned int tex_fbo;

	unsigned int mat_vao;
	unsigned int mat_tex_vbo;
	unsigned int mat_pos_vbo;
	unsigned int debug_vbo;

	unsigned int screen_quad;

	unsigned int writer_shader_program;
	unsigned int screen_shader_program;

	unsigned int material_texture;
	unsigned int write_texture;

	Arena* arena;

	void compile_shaders(unsigned int world_width, unsigned int world_height);

public:
	void draw_empty_rect(const std::vector<glm::mat4> debug_rect_transforms, float offset);
	void init_render_data(unsigned int world_width, unsigned int world_height);
	void draw_materials(const std::vector<float>& tex_offsets, const std::vector<glm::mat4>& positions, const vector2& size);
	void screen_render();
	~Renderer();
};


#endif
