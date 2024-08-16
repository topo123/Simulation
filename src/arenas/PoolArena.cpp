#include "PoolArena.hpp"
#include <sys/mman.h>
#include <cassert>



PoolArena* init_pool(size_t num_blocks, size_t block_size)
{
	PoolArena* arena = new PoolArena();
	arena->block_size = block_size;
	arena->arena_size = num_blocks * block_size;
	arena->arena_ptr = mmap(nullptr, arena->arena_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); 
	return arena;
}


void* allocate(PoolArena* arena)
{
	size_t allocate_pos = arena->offset + arena->block_size;

	if(allocate_pos > arena->arena_size)
	{
		return nullptr;
	}

	if(arena->list != nullptr)
	{
		FreeList* head = arena->list;
		void* mem_block = head->mem_block;
		arena->list = head->next_node;
		delete head;
		return mem_block;
	}

	void* mem_ptr = static_cast<char*>(arena->arena_ptr) + arena->offset;
	arena->offset = allocate_pos;
	return mem_ptr;
}

int deallocate(PoolArena* arena, void* mem_block)
{
	if(mem_block < arena->arena_ptr || (mem_block > (static_cast<char*>(arena->arena_ptr) + arena->arena_size)))
	{
		return -1;
	}
	FreeList* head = arena->list;
	arena->list = new FreeList();
	arena->list->mem_block = mem_block;
	arena->list->next_node = head;
	return 0;
}

int free_arena(PoolArena* arena)
{
	int free_failure = munmap(arena->arena_ptr, arena->arena_size);
	assert(free_failure == 0);

	FreeList* list = arena->list;
	while(list != nullptr)
	{
		FreeList* next_node = list;
		list = list->next_node;
		delete next_node;
	}

	delete arena;
	return free_failure;
}
