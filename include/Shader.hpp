#ifndef SHADER_H
#define SHADER_H
#include <string>
#include "Arena.hpp"

struct Shader
{
	char* frag_shader;
	char* vertex_shader;
};

Shader* get_shader(std::string vertex, std::string frag, Arena* shader_arena);
void create_shader_program(unsigned int& shader_program, const std::string vshader_path, const std::string fshader_path);
void print_shader(char* shader);



#endif
