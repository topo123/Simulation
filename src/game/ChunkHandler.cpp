#include <ChunkHandler.hpp>
#include <cstdint>

void ChunkHandler::init_chunk_handler(uint16_t cW, uint16_t cH, uint16_t wW, uint16_t wH)
{
	if(wW % cW != 0 || wH % cH != 0)
	{
		return;
	}

	chunk_width = cW;
	chunk_height = cH;

	x_chunks = wW/cW;
	y_chunks = wH/cH;
	chunks.resize(x_chunks * y_chunks, new Chunk());

	uint16_t x = 0, y = 0;
	for(size_t i = 0; i < chunks.size(); i ++)
	{
		Chunk* chunk = chunks[i];
		chunk->particles = new Material[chunk_width * chunk_height];
		chunk->x_coord = x;
		chunk->y_coord = y;

		x += chunk_width;
		if(x == wW)
		{
			x = 0;
			y ++;
		}
	}
}

void ChunkHandler::add_material(Material* material)
{
	uint16_t x = material->position.first;
	uint16_t y = material->position.second;

	size_t chunk_pos = (y/y_chunks) * y_chunks + (x/x_chunks); 
	assert(chunk_pos >= 0 && chunk_pos < chunks.size());
	Chunk* chunk = chunks[chunk_pos];
	set_material(chunk->particles[index(x, y)], material);
}

size_t ChunkHandler::index(uint16_t x, uint16_t y)
{
	return x + y * chunk_width;
};
