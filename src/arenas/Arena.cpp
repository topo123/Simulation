#include <windows.h>
#include <iostream>
#include <ostream>
#ifdef linux
#include <iostream>
#include <sys/mman.h>
#endif
#ifdef WIN32
#include <memoryapi.h>
#endif
#include <cassert>
#include <Arena.hpp>

void CheckVirtualAllocParameters(size_t arena_size) {
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);

	if (arena_size == 0) {
		std::cerr << "Error: arena_size is zero." << std::endl;
		return;
	}

	if (arena_size % sysInfo.dwAllocationGranularity != 0) {
		std::cerr << "Error: arena_size is not a multiple of allocation granularity." << std::endl;
		std::cerr << "Allocation Granularity: " << sysInfo.dwAllocationGranularity << std::endl;
		return;
	}

	// VirtualAlloc call for testing
	unsigned char* arena_ptr = static_cast<unsigned char*>(VirtualAlloc(NULL, arena_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	if (arena_ptr == NULL) {
		DWORD error = GetLastError();
		std::cerr << "VirtualAlloc failed with error code: " << error << std::endl;
		return;
	}

	std::cout << "VirtualAlloc succeeded." << std::endl;
	VirtualFree(arena_ptr, 0, MEM_RELEASE);
}


Arena* init_arena(size_t arena_size)
{
	CheckVirtualAllocParameters(arena_size);
	Arena* arena = new Arena();
#ifdef WIN32
	arena->arena_ptr = static_cast<unsigned char*>(VirtualAlloc(NULL, arena_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	//unsigned char* arena_ptr = static_cast<unsigned char*>(VirtualAlloc(NULL, arena_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
	DWORD error_code = GetLastError();


	// Allocate a buffer for the error message
	char* error_message = nullptr;

	// Use FormatMessage to get a readable string for the error code
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		error_code,
		0, // Language ID (0 = default)
		(LPSTR)&error_message,
		0,
		NULL
	);

	// Print the error code and the corresponding message
	std::cout << "Error Code: " << error_code << std::endl;
	std::cout << "Error Message: " << error_message << std::endl;

	// Free the allocated memory for the error message
	LocalFree(error_message);
	assert(arena->arena_ptr != nullptr);
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

#ifdef WIN32
	std::cout << "Freeing the shader memory" << std::endl;
	int free_failure = VirtualFree(arena->arena_ptr, 0, MEM_RELEASE);
	assert(free_failure == true);
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

