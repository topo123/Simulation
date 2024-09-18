#ifndef CHUNKHANNDLER_H
#define CHUNKHANNDLER_H
#include <Logger.hpp>
#include <string>
#include <Renderer.hpp>
#include <PoolArena.hpp>
#include <Material.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class ChunkHandler
{

	int x_chunks;
	int y_chunks;


	int world_width;
	int world_height;
	Logger logger;
	Logger dirty;
	Logger mat_logger;


public:
	int chunk_size;

	struct Move
	{
		vector2 old_pos;
		vector2 new_pos;
	};

	struct Chunk
	{
		vector2 d_upper{-1,-1};
		vector2 d_lower{-1, -1};
		vector2 coords {0, 0};
		int num_materials {0};
		int static_materials {0};
		std::unordered_set<Material*> update_list;
		std::vector<Material*> materials;
		std::vector<Move> moves;
	};

	void make_dirty_rect(Chunk* chunk);
	void expand_rect(Chunk* chunk, vector2* old_pos, vector2* new_pos);

	Chunk* get_chunk(int x, int y);
	void commit_changes(Chunk* chunk);
	void log(std::string message);
	bool in_world(int x, int y);
	bool in_rect(Chunk* chunk, vector2* pos);
	std::string print_pos(int x, int y);
	void init_chunk_handler(int chunk_width, int chunk_height, int world_width,  int world_height);
	Chunk* init_chunk(int mat_x, int mat_y);
	void add_materials(const std::vector<Material*>& materials, PoolArena* arena);
	void update_chunk(Chunk* chunk);
	void draw_chunk(Chunk* chunk, Renderer render);
	size_t index(int x, int y);
	Material* get_material(int x, int y);

	Chunk* move_material(Chunk* chunk, Material* material, vector2* old_pos, vector2* new_pos);
	bool update_down(Chunk* chunk, Material* material);
	bool update_side(Chunk* chunk, Material* material);
	bool update_side_down(Chunk* chunk, Material* material);

	int chunk_width;
	int chunk_height;
	std::unordered_map<vector2, Chunk*, vector_hash> chunks;
	std::vector<Chunk*> iter_chunks;
};
#endif
