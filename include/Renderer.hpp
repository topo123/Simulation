#ifndef RENDERER_H
#define RENDERER_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <Shader.hpp>
#include <Arena.hpp>
#include <GameState.hpp>

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
	void render(float tex_offset, glm::vec2 pos, glm::vec2 size);
};

void draw_materials(Renderer& drawer, MaterialState* state);

#endif
