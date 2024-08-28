#ifndef CHUNKHANNDLER_H
#define CHUNKHANNDLER_H
#include <Renderer.hpp>
#include <PoolArena.hpp>
#include <Material.hpp>
#include <map>
#include <vector>

class ChunkHandler
{

	uint16_t x_chunks;
	uint16_t y_chunks;

	uint16_t chunk_width;
	uint16_t chunk_height;

	uint16_t world_width;
	uint16_t world_height;


public:
	uint16_t chunk_size;
	struct Chunk
	{
		std::pair<uint16_t, uint16_t> coords;
		int num_materials = 0;
		Material** particles;
	};

	void init_chunk_handler(uint16_t chunk_width, uint16_t chunk_height, uint16_t world_width,  uint16_t world_height);
	void add_materials(Material** material, size_t num_materials, PoolArena* arena);
	void update_chunk(Chunk* chunk);
	void draw_chunk(Chunk* chunk, Renderer render);
	size_t index(uint16_t x, uint16_t y);
	Material* get_material(size_t x, size_t y);
	void set_null(Material** material);
	bool correct(Material** material);

	Chunk* move_material(Chunk* chunk, Material* material, std::pair<uint16_t, uint16_t> old_pos, std::pair<uint16_t, uint16_t> new_pos);
	void update_down(Chunk* chunk, Material* material);
	void update_side(Chunk* chunk, Material* material);
	void update_side_down(Chunk* chunk, Material* material);

	std::map<std::pair<uint16_t, uint16_t>, Chunk*> chunks;
	std::vector<Chunk*> iter_chunks;
};

#endif
