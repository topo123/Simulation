#include "PoolArena.hpp"
#ifdef linux
#include <sys/mman.h>
#endif
#ifdef WIN32
#include <memoryapi.h>
#endif
#include <cassert>



PoolArena* init_pool(size_t num_blocks, size_t block_size)
{
	PoolArena* arena = new PoolArena();
	arena->block_size = block_size;
	arena->arena_size = num_blocks * block_size;
#ifdef WIN32
	arena->arena_ptr = VirtualAlloc(NULL, arena->arena_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE); 
#endif

#ifdef linux
	arena->arena_ptr = static_cast<unsigned char*>(mmap(NULL, arena->arena_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
	assert(arena->arena_ptr != MAP_FAILED);
#endif
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
		void* mem_block = (void*)head;
		arena->list = head->next_node;
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
	arena->list = (FreeList*)mem_block;
	arena->list->next_node = head;
	return 0;
}

int free_arena(PoolArena* arena)
{
#ifdef linux
	int free_failure = munmap(arena->arena_ptr, arena->arena_size);
	assert(free_failure == 0);
#endif

#ifdef WIN32
	int free_failure = VirtualFree(arena->arena_ptr, 0, MEM_RELEASE);
	assert(free_failure == 0);
#endif

	delete arena;
	return free_failure;
}
