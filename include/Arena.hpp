#ifndef ARENA_H
#define ARENA_H
#include <cstddef>

struct Arena
{
	void* arena_ptr;
	size_t arena_size;
	size_t offset;
};

Arena* init_arena(size_t arena_size = 1024 * 1024 * 1024);
int free_arena(Arena* arena);
void* allocate(Arena* arena, size_t allocate_size);

#endif
