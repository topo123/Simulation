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
void print_shader(char* shader);



#endif
