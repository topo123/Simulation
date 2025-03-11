#include <iostream>
#include <fstream>
#include <string>
#include "Shader.hpp"


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

