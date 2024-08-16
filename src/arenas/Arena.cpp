#include <cassert>
#include <sys/mman.h>
#include <iostream>
#include "Arena.hpp"



Arena* init_arena(size_t arena_size)
{
	Arena* arena = new Arena();
	arena->arena_ptr = static_cast<unsigned char*>(mmap(nullptr, arena_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
	arena->offset = 0;
	arena->arena_size = arena_size;
	return arena;
}

int free_arena(Arena* arena)
{
	int free_failure = munmap(arena->arena_ptr, arena->arena_size);
	assert(free_failure == 0);

	if(free_failure == -1)
	{
		delete arena;
	}
	return free_failure;
}

void* allocate(Arena* arena, size_t allocate_size)
{
	size_t alloc_end_pos = arena->offset + allocate_size;
	
	
	if(alloc_end_pos > arena->arena_size)
	{
		return nullptr;
	}

	std::cout << "Offset End pos: " << std::to_string(alloc_end_pos) << '\n';
	std::cout << "Current arena size: " << std::to_string(arena->arena_size) << " Current Offset: : " << std::to_string(arena->offset) << '\n';

	void* alloc_pos = static_cast<char*>(arena->arena_ptr) + arena->offset;
	arena->offset = alloc_end_pos;
	return alloc_pos;
}

