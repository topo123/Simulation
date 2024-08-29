#ifndef CHUNKHANNDLER_H
#define CHUNKHANNDLER_H
#include <Renderer.hpp>
#include <PoolArena.hpp>
#include <Material.hpp>
#include <map>
#include <vector>

class ChunkHandler
{

	int x_chunks;
	int y_chunks;

	int chunk_width;
	int chunk_height;

	int world_width;
	int world_height;


public:
	int chunk_size;
	struct Chunk
	{
		vector2 coords {0, 0};
		int num_materials = 0;
		std::vector<Material*> materials;
	};

	void init_chunk_handler(int chunk_width, int chunk_height, int world_width,  int world_height);
	Chunk* init_chunk(int mat_x, int mat_y);
	void add_materials(const std::vector<Material*>& materials, PoolArena* arena);
	void update_chunk(Chunk* chunk);
	void draw_chunk(Chunk* chunk, Renderer render);
	size_t index(int x, int y);
	Material* get_material(size_t x, size_t y);

	Chunk* move_material(Chunk* chunk, Material* material, vector2* old_pos, vector2* new_pos);
	void update_down(Chunk* chunk, Material* material);
	void update_side(Chunk* chunk, Material* material);
	void update_side_down(Chunk* chunk, Material* material);

	std::map<vector2, Chunk*> chunks;
	std::vector<Chunk*> iter_chunks;
};
#endif
