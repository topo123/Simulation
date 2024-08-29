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
	std::cout << "Shader compilation broken" << '\n';
	arena = init_arena();
	shader = get_shader("src/shaders/vertex.vert", "src/shaders/fragment.frag", arena);


	unsigned int vShader = glCreateShader(GL_VERTEX_SHADER);
	std::cout << "Error here" << '\n';
	glShaderSource(vShader, 1, &(shader->vertex_shader), NULL);
	std::cout << "Not reached" << '\n';
	glCompileShader(vShader);

	int success;
	char infoLog[512];
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	unsigned int fShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fShader, 1, &(shader->frag_shader), NULL);
	glCompileShader(fShader);

	glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	shader_program = glCreateProgram();
	glAttachShader(shader_program, vShader);
	glAttachShader(shader_program, fShader);
	glLinkProgram(shader_program);


	glDeleteShader(vShader);
	glDeleteShader(fShader);

	glUseProgram(shader_program);
	glm::mat4 proj = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);
	int matrix = glGetUniformLocation(shader_program, "projection");
	assert(matrix != -1);
	glUniformMatrix4fv(matrix, 1, GL_FALSE, glm::value_ptr(proj));
}

void Renderer::initRenderData()
{
	unsigned char tex_colors[] = {
		246, 220, 189, 255, //sand color
		15, 94, 156, 255, //water color
	};

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_colors);


	compile_shaders();
	unsigned int VBO;
	unsigned int EBO;

	float vertices[] = {
		0.0f, 0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};

	unsigned int indices[] = {
		0, 1, 2,
		1, 2, 3
	};

	glGenVertexArrays(1, &quad);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);


	glBindVertexArray(quad);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); 
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


void Renderer::render(float tex_offset, vector2* pos, vector2* size)
{
	glUseProgram(shader_program);
	glm::mat4 model = glm::mat4(1.0f);

	model = glm::translate(model, glm::vec3(pos->x, pos->y, 0.0f));
	model = glm::scale(model, glm::vec3(size->x, size->y, 1.0f));

	int matrix = glGetUniformLocation(shader_program, "model");
	assert(matrix != -1);
	glUniformMatrix4fv(matrix, 1, GL_FALSE, glm::value_ptr(model));


	int off = glGetUniformLocation(shader_program,"offset"); 
	assert(off != -1);
	glUniform1f(off, tex_offset);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glBindVertexArray(quad);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}


