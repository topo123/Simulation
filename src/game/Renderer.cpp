#include <cassert> 
#include <World.hpp>
#include <iostream>
#include <glad/glad.h>
#include <Renderer.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Shader.hpp>


void Renderer::compile_shaders()
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
	glm::mat4 proj = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);
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

void Renderer::initRenderData()
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

	float vertices[] = {
		0.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f
	};

	unsigned int indices[] = {
		0, 1, 2,
		0, 3, 2
	};

	float write_vertices[] = {
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		1.0f, -1.0f,  1.0f, 0.0f,
		1.0f,  1.0f,  1.0f, 1.0f
	};


	compile_shaders();
	unsigned int VBO;
	unsigned int EBO;
	unsigned int write_vbo;


	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);


	glGenTextures(1, &material_texture);
	glBindTexture(GL_TEXTURE_2D, material_texture);

	GLint swizzle_mask[] = {GL_RED, GL_GREEN, GL_BLUE, GL_ONE};
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle_mask);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 12, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_colors);


	glGenTextures(1, &write_texture);
	glBindTexture(GL_TEXTURE_2D, write_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, write_texture, 0);

	glGenVertexArrays(1, &write_texture_quad);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(write_texture_quad);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); 
	glEnableVertexAttribArray(0);

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
	glBufferData(GL_ARRAY_BUFFER, sizeof(write_vertices), write_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,  4 * sizeof(float), (void*)0); 
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}


void Renderer::texture_render(float tex_offset, vector2* pos, vector2* size)
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glUseProgram(writer_shader_program);
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, glm::vec3(pos->x, pos->y, 0.0f));
	model = glm::scale(model, glm::vec3(size->x, size->y, 1.0f));

	int matrix = glGetUniformLocation(writer_shader_program, "model");
	assert(matrix != -1);
	glUniformMatrix4fv(matrix, 1, GL_FALSE, glm::value_ptr(model));


	int off = glGetUniformLocation(writer_shader_program,"offset"); 
	assert(off != -1);
	glUniform1f(off, tex_offset);

	glBindTexture(GL_TEXTURE_2D, material_texture);
	glBindVertexArray(write_texture_quad);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}



void Renderer::draw_texture_rect(vector2& upper, vector2& lower, float offset)
{
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glUseProgram(writer_shader_program);
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, glm::vec3(upper.x, upper.y, 0.0f));
	model = glm::scale(model, glm::vec3((lower.x - upper.x), (lower.y - upper.y), 1.0f));

	int matrix = glGetUniformLocation(writer_shader_program, "model");
	assert(matrix != -1);
	glUniformMatrix4fv(matrix, 1, GL_FALSE, glm::value_ptr(model));


	int off = glGetUniformLocation(writer_shader_program,"offset"); 
	assert(off != -1);
	glUniform1f(off, offset);

	glBindTexture(GL_TEXTURE_2D, material_texture);
	glBindVertexArray(write_texture_quad);
	glDrawArrays(GL_LINE_LOOP, 0, 4);
	glBindVertexArray(0);
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

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glClearColor(87.0f/255.0f, 88.0f/255.0f, 87.0f/255.0f, 0.1f);
	glClear(GL_COLOR_BUFFER_BIT);
}


Renderer::~Renderer()
{
	glDeleteTextures(1 ,&material_texture);
	glDeleteVertexArrays(1, &write_texture_quad);
	glDeleteShader(screen_shader_program);
}
