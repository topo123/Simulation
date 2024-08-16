#ifndef CHUNKHANNDLER_H
#define CHUNKHANNDLER_H
#include <Material.hpp>
#include <vector>

class ChunkHandler
{

	uint16_t x_chunks;
	uint16_t y_chunks;

	uint16_t chunk_width;
	uint16_t chunk_height;

	void set_material(Material* mat1, Material* mat2);

public:
	struct Chunk
	{
		uint16_t x_coord;
		uint16_t y_coord;
		Material* particles {nullptr};
	};

	std::vector<Chunk*> chunks;

	void init_chunk_handler(uint16_t chunk_width, uint16_t chunk_height, uint16_t world_width, uint16_t world_height);
	void add_material(Material* material);
	void update_chunk(Chunk* material);
	size_t index(uint16_t x, uint16_t y);
	


};

#endif
