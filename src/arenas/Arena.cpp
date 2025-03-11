#ifdef linux
#include <sys/mman.h>
#endif
#include <cassert>
#include <Arena.hpp>



Arena* init_arena(size_t arena_size)
{
	Arena* arena = new Arena();
#ifdef WIN32
	arena->arena_ptr = static_cast<unsigned char*>(VirtualAlloc(NULL, arena->arena_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
#endif

#ifdef linux
	arena->arena_ptr = static_cast<unsigned char*>(mmap(NULL, arena_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
#endif
	arena->offset = 0;
	arena->arena_size = arena_size;
	return arena;
}

int free_arena(Arena* arena)
{
#ifdef linux
	int free_failure = munmap(arena->arena_ptr, arena->arena_size);
	assert(free_failure == 0);
#endif

	if(free_failure == 0)
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

	void* alloc_pos = static_cast<char*>(arena->arena_ptr) + arena->offset;
	arena->offset = alloc_end_pos;
	return alloc_pos;
}

