#include <cstdint>
#include <ChunkHandler.hpp>

class World
{
	ChunkHandler handler;
	


public:
	uint16_t world_width;
	uint16_t world_height;

	void init_world(uint16_t chunk_width, uint16_t chunk_height, uint16_t world_width, uint16_t world_height);
	void update_world();
};
