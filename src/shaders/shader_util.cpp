#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <string>
#include "Shader.hpp"

void create_shader_program(unsigned int& shader_program, std::string vshader_path, std::string fshader_path)
{
	Arena* arena = init_arena(1024);
	Shader* shader = get_shader(vshader_path, fshader_path, arena);

	unsigned int vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshader, 1, &(shader->vertex_shader), NULL);
	glCompileShader(vshader);

	int success;
	char infoLog[1024];
	glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vshader, 1024, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED texture shader\n" << infoLog << std::endl;
		throw std::bad_exception();
	}

	unsigned int fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshader, 1, &(shader->frag_shader), NULL);
	glCompileShader(fshader);

	glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fshader, 1024, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED texture shader\n" << infoLog << std::endl;
		throw std::bad_exception();
	}

	shader_program = glCreateProgram();
	glAttachShader(shader_program, vshader);
	glAttachShader(shader_program, fshader);
	glLinkProgram(shader_program);

	glDeleteShader(vshader);
	glDeleteShader(fshader);

	free_arena(arena);
}

void print_shader(char* shader) 
{
	for(char *c = shader; (*c) != '\0'; c ++)
	{
		std::cout << (*c);
	}
}

Shader* get_shader(std::string vertex, std::string frag, Arena* shader_arena)
{
	std::ifstream frag_reader(frag);
	std::ifstream vert_reader(vertex);
	Shader* shader {new Shader()};

	if(!frag_reader || !vert_reader)
	{
		return nullptr;
	}

	char c {0};
	

	size_t frag_size = 0;
	size_t vert_size = 0;

	while(frag_reader.get(c))
	{
		frag_size ++;
	}

	while(vert_reader.get(c))
	{
		vert_size ++;
	}

	frag_size ++;
	vert_size ++;


	shader->frag_shader = static_cast<char*>(allocate(shader_arena, frag_size));
	shader->vertex_shader = static_cast<char*>(allocate(shader_arena, vert_size)); 

	if(shader->frag_shader == nullptr || shader->vertex_shader == nullptr)
	{
		return nullptr;
	}

	size_t frag_pos = 0;
	size_t vert_pos = 0;

	frag_reader.clear();
	frag_reader.seekg(0);

	vert_reader.clear();
	vert_reader.seekg(0);

	while(frag_reader.get(c))
	{
		shader->frag_shader[frag_pos] = c;
		frag_pos ++;
	}

	while(vert_reader.get(c))
	{
		shader->vertex_shader[vert_pos] = c;
		vert_pos ++;
	}
	
	shader->vertex_shader[vert_size - 1] = '\0';
	shader->frag_shader[frag_size - 1] = '\0';
	print_shader(shader->vertex_shader);
	print_shader(shader->frag_shader);

	return shader;
}

