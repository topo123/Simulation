#ifndef POOLARENA_H
#define POOLARENA_H
#include <cstddef>

struct FreeList
{
	void* mem_block {nullptr};
	FreeList* next_node {nullptr};
};

struct PoolArena
{
	FreeList* list{nullptr};
	void* arena_ptr {nullptr};
	size_t offset {0};
	size_t block_size {0};
	size_t arena_size {0};
};

PoolArena* init_pool(size_t num_blocks, size_t block_size);
void* allocate(PoolArena* arena);
int deallocate(PoolArena* arena, void* mem_block);
int free_arena(PoolArena* arena);
#endif
