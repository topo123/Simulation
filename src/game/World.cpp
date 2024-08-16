#include <World.hpp>

void World::init_world(uint16_t cW, uint16_t cH, uint16_t wW, uint16_t wH)
{
	world_width = wW;
	world_height = wH;
	
	handler.init_chunk_handler(cW,cH, wW, wH);
}


void World::update_world()
{
	for(ChunkHandler::Chunk* chunk: handler.chunks)
	{
	}
}

