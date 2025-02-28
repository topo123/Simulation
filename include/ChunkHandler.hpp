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
	PoolArena* material_arena;
	MatTexCoords tex_coords;


public:
	int chunk_size;

	struct Move
	{
		vector2 old_pos;
		vector2 new_pos;
	};

	struct Chunk
	{
		std::vector<Material*> materials;
		std::unordered_set<Material*> update_list;
		std::vector<Move> move_list;
		std::vector<Move> swap_list;
		vector2 d_upper{-1,-1};
		vector2 d_lower{-1, -1};
		vector2 coords {0, 0};
		int asleep;
		int num_materials {0};
	};

	vector2 get_rxn_coord(Material* material);
	std::string print_pos(int x, int y);
	size_t index(int x, int y);
	Material* get_material(int x, int y);

	void acid_mat_rxn(Chunk* chunk, Material* water, Material* other);
	void water_sand_rxn(Chunk* chunk, Material* water, Material* sand);
	void smoke_falling_rxn(Chunk* chunk, Material* smoke, Material* other);
	void smoke_mat_rxn(Chunk* chunk, Material* smoke, Material* other);
	void destroy_material(Material* material);
	void make_dirty_rect(Chunk* chunk);
	void modify_rect(Chunk* chunk, vector2* old_pos, vector2* new_pos);
	void commit_changes(Chunk* chunk);
	void log(std::string message);
	void init_chunk_handler(int chunk_width, int chunk_height, int world_width,  int world_height, PoolArena* arena);
	void add_materials(const std::vector<Material*>& materials);
	void update_chunk(Chunk* chunk);
	void draw_chunk(Chunk* chunk, Renderer* render);
	bool can_react(Material* m1, Material* m2);
	void react(Chunk* chunk, Material* m1, Material* m2);
	void move_material(Chunk* chunk, Material* material, vector2* old_pos, vector2* new_pos);
	void swap_material(Chunk* chunk, Material* material, vector2* old_pos, vector2* new_pos);

	Chunk* get_chunk(int x, int y);
	Chunk* init_chunk(int mat_x, int mat_y);

	bool in_world(int x, int y);
	bool update_down(Chunk* chunk, Material* material);
	bool update_side(Chunk* chunk, Material* material);
	bool update_side_down(Chunk* chunk, Material* material);
	bool update_up(Chunk* chunk, Material* material);
	bool update_side_up(Chunk* chunk, Material* material);
	~ChunkHandler();

	int chunk_width;
	int chunk_height;
	std::unordered_map<vector2, Chunk*, vector_hash> chunks;
	std::vector<Chunk*> iter_chunks;
	std::vector<Material*> trash_materials;
};
#endif
