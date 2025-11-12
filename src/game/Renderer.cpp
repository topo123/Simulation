#include <cassert> 
#include <World.hpp>
#include <iostream>
#include <glad/glad.h>
#include <Renderer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Shader.hpp>

void Renderer::init_render_data(unsigned int world_width, unsigned int world_height)
{
	create_shader_program(writer_shader_program, "shaders/material_vert_shader.glsl", "shaders/material_frag_shader.glsl");
	create_shader_program(debug_shader_program, "shaders/debug_vert_shader.glsl", "shaders/debug_frag_shader.glsl");
	create_shader_program(screen_shader_program, "shaders/screen_vert_shader.glsl", "shaders/screen_frag_shader.glsl");


	glm::mat4 proj = glm::ortho(0.0f, (float)world_width, (float)world_height, 0.0f, -1.0f, 1.0f);

	glUseProgram(writer_shader_program);
	int matrix = glGetUniformLocation(writer_shader_program, "projection");
	assert(matrix != -1);
	glUniformMatrix4fv(matrix, 1, GL_FALSE, glm::value_ptr(proj));

	glUseProgram(debug_shader_program);
	matrix = glGetUniformLocation(writer_shader_program, "projection");
	assert(matrix != -1);
	glUniformMatrix4fv(matrix, 1, GL_FALSE, glm::value_ptr(proj));

	unsigned char tex_colors[] = {
		246, 220, 189, 255, //sand color
		15, 94, 156, 255, //water color
		136, 140, 141, 255, //stone color
		102, 255, 102, 255, //acid color
		132, 147, 156, 50, //smoke color
		79, 48, 31, 255,   //wood color
		255, 99, 0, 255,   //fire color
		255, 0, 0, 255,    //change fire color
		44, 36, 22, 255,   //oil color
		128, 191, 77, 10,   //flammable gas color
		170, 255, 0, 255,  //dirty rect debug color
		255, 0, 0, 255 //grid debug color
	};

	float quad_vertices[] = {
		0.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f
	};


	float empty_quad_vertices[] = {
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};

	unsigned int empty_quad_indices[] = {
		0, 1,
		1, 2,
		2, 3,
		0, 3
	};

	unsigned int indices[] = {
		0, 1, 2,
		0, 3, 2
	};

	float texture_vertices[] = {
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f
	};

	unsigned int empty_quad_data;
	unsigned int empty_quad_indices_data;
	unsigned int mat_quad_ebo;
	unsigned int mat_quad_vbo;
	unsigned int write_vbo;


	glGenFramebuffers(1, &tex_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, tex_fbo);

	glGenTextures(1, &material_texture);
	glBindTexture(GL_TEXTURE_1D, material_texture);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 12, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_colors);

	glGenTextures(1, &write_texture);
	glBindTexture(GL_TEXTURE_2D, write_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, world_width, world_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, write_texture, 0);

	glGenVertexArrays(1, &mat_vao);
	glGenBuffers(1, &mat_tex_vbo);
	glGenBuffers(1, &mat_pos_vbo);
	glGenBuffers(1, &debug_vbo);
	glGenBuffers(1, &mat_quad_vbo);
	glGenBuffers(1, &mat_quad_ebo);

	glGenVertexArrays(1, &debug_vao);
	glGenBuffers(1, &empty_quad_data);
	glGenBuffers(1, &empty_quad_indices_data);

	glBindVertexArray(mat_vao);

	glBindBuffer(GL_ARRAY_BUFFER, mat_quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mat_quad_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); 
	glEnableVertexAttribArray(0);


	glBindBuffer(GL_ARRAY_BUFFER, mat_tex_vbo);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
	glVertexAttribDivisor(1, 1);
	glEnableVertexAttribArray(1);


	glBindBuffer(GL_ARRAY_BUFFER, mat_pos_vbo);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));


	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);

	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);

	glBindVertexArray(debug_vao);
	glBindBuffer(GL_ARRAY_BUFFER, empty_quad_data);
	glBufferData(GL_ARRAY_BUFFER, sizeof(empty_quad_vertices), empty_quad_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, empty_quad_indices_data);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(empty_quad_indices), empty_quad_indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, debug_vbo);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);

	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glGenVertexArrays(1, &screen_quad);
	glGenBuffers(1, &write_vbo);
	glBindVertexArray(screen_quad);

	glBindBuffer(GL_ARRAY_BUFFER, write_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texture_vertices), texture_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,  4 * sizeof(float), (void*)0); 
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

}


void Renderer::draw_materials(const std::vector<float>& tex_offsets, const std::vector<glm::mat4>& positions, const vector2& size)
{
	glBindFramebuffer(GL_FRAMEBUFFER, tex_fbo);
	glUseProgram(writer_shader_program);
	glBindTexture(GL_TEXTURE_1D, material_texture);

	glBindVertexArray(mat_vao);

	glBindBuffer(GL_ARRAY_BUFFER, mat_tex_vbo);
	glBufferData(GL_ARRAY_BUFFER, tex_offsets.size() * sizeof(float), tex_offsets.data(), GL_DYNAMIC_DRAW);


	glBindBuffer(GL_ARRAY_BUFFER, mat_pos_vbo);
	glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::mat4), positions.data(), GL_DYNAMIC_DRAW);

	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, positions.size());

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}



void Renderer::draw_empty_rect(const std::vector<glm::mat4> debug_rect_transforms, float offset)
{
	glBindFramebuffer(GL_FRAMEBUFFER, tex_fbo);
	glUseProgram(debug_shader_program);
	glBindTexture(GL_TEXTURE_1D, material_texture);

	glBindVertexArray(debug_vao);

	int tex_offset = glGetUniformLocation(debug_shader_program, "tex_offset");
	glUniform1f(tex_offset, offset);

	glBindBuffer(GL_ARRAY_BUFFER, debug_vbo);
	glBufferData(GL_ARRAY_BUFFER, debug_rect_transforms.size() * sizeof(glm::mat4), debug_rect_transforms.data(), GL_DYNAMIC_DRAW);


	glDrawElementsInstanced(GL_LINES, 8, GL_UNSIGNED_INT, 0, debug_rect_transforms.size());
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Renderer::screen_render()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(87.0f/255.0f, 88.0f/255.0f, 87.0f/255.0f, 0.1f);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(screen_shader_program);
	glBindTexture(GL_TEXTURE_2D, write_texture);

	glBindVertexArray(screen_quad);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, tex_fbo);
	glClearColor(87.0f/255.0f, 88.0f/255.0f, 87.0f/255.0f, 0.1f);
	glClear(GL_COLOR_BUFFER_BIT);
}


Renderer::~Renderer()
{
	glDeleteTextures(1 ,&material_texture);
	glDeleteVertexArrays(1, &mat_vao);
	glDeleteShader(screen_shader_program);
}
