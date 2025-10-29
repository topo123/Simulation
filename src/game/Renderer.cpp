#include <cassert> 
#include <World.hpp>
#include <iostream>
#include <glad/glad.h>
#include <Renderer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Shader.hpp>


void Renderer::compile_shaders(unsigned int world_width, unsigned int world_height)
{
	std::cout << "Initializing the arena\n";
	arena = init_arena(1024 * 1024 * 1024);
	std::cout << "Arena initialized\n";

	Shader* shader = get_shader("shaders/frame_vert.vert", "shaders/frame_frag.frag", arena);
	Shader* fbo_shader = get_shader("shaders/fbo_vert.vert", "shaders/fbo_frag.frag", arena);


	unsigned int vShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vShader, 1, &(shader->vertex_shader), NULL);
	glCompileShader(vShader);

	int success;
	char infoLog[1024];
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vShader, 1024, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED texture shader\n" << infoLog << std::endl;
		throw std::bad_exception();
	}

	unsigned int fShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fShader, 1, &(shader->frag_shader), NULL);
	glCompileShader(fShader);

	glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fShader, 1024, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED texture shader\n" << infoLog << std::endl;
		throw std::bad_exception();
	}

	writer_shader_program = glCreateProgram();
	glAttachShader(writer_shader_program, vShader);
	glAttachShader(writer_shader_program, fShader);
	glLinkProgram(writer_shader_program);


	std::cout << "Deleting the shaders\n";
	glDeleteShader(vShader);
	glDeleteShader(fShader);
	std::cout << "Shaders deleted\n";

	glUseProgram(writer_shader_program);
	glm::mat4 proj = glm::ortho(0.0f, (float)world_width, (float)world_height, 0.0f, -1.0f, 1.0f);
	int matrix = glGetUniformLocation(writer_shader_program, "projection");
	assert(matrix != -1);
	glUniformMatrix4fv(matrix, 1, GL_FALSE, glm::value_ptr(proj));

	unsigned int fbo_vShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(fbo_vShader, 1, &(fbo_shader->vertex_shader), NULL);
	glCompileShader(fbo_vShader);

	int fbo_success;
	char fbo_infoLog[1024];
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &fbo_success);
	if (!fbo_success)
	{
		glGetShaderInfoLog(vShader, 1024, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED screen shader\n" << infoLog << std::endl;
		throw std::bad_exception();
	}

	unsigned int fbo_fShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fbo_fShader, 1, &(fbo_shader->frag_shader), NULL);
	glCompileShader(fbo_fShader);

	glGetShaderiv(fbo_fShader, GL_COMPILE_STATUS, &fbo_success);
	if (!fbo_success)
	{
		glGetShaderInfoLog(fbo_fShader, 1024, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED screen shader\n" << infoLog << std::endl;
		throw std::bad_exception();
	}

	screen_shader_program = glCreateProgram();
	glAttachShader(screen_shader_program, fbo_vShader);
	glAttachShader(screen_shader_program, fbo_fShader);
	glLinkProgram(screen_shader_program);


	std::cout << "Deleting the shaders\n";
	glDeleteShader(fbo_vShader);
	glDeleteShader(fbo_fShader);
	glDeleteShader(vShader);
	glDeleteShader(fShader);
	std::cout << "Shaders deleted\n";



	std::cout << "Freeing the arean\n";
	free_arena(arena);
	std::cout << "Freed the arean\n";
}

void Renderer::init_render_data(unsigned int world_width, unsigned int world_height)
{
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


	compile_shaders(world_width, world_height);
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
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 12, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_colors);

	unsigned char pixels[12 * 4];
	glGetTexImage(GL_TEXTURE_1D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	for(size_t i = 0; i < 48; i ++)
	{
		 std::cout << (int)pixels[i] << ", ";
	}
	std::cout << "\n";

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


	glBindBuffer(GL_ARRAY_BUFFER, debug_vbo);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

	glVertexAttribDivisor(6, 1);
	glVertexAttribDivisor(7, 1);
	glVertexAttribDivisor(8, 1);
	glVertexAttribDivisor(9, 1);

	glEnableVertexAttribArray(6);
	glEnableVertexAttribArray(7);
	glEnableVertexAttribArray(8);
	glEnableVertexAttribArray(9);


	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);


	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Error: Framebuffer is not complete!" << std::endl;
	}

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
	glUseProgram(writer_shader_program);
	glBindTexture(GL_TEXTURE_1D, material_texture);

	glBindVertexArray(mat_vao);

	glBindBuffer(GL_ARRAY_BUFFER, debug_vbo);
	glBufferData(GL_ARRAY_BUFFER, debug_rect_transforms.size() * sizeof(glm::mat4), debug_rect_transforms.data(), GL_DYNAMIC_DRAW);

	glDrawArraysInstanced(GL_LINE_LOOP, 0, 4, debug_rect_transforms.size());
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
