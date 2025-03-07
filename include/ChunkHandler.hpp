#ifndef CHUNKHANNDLER_H
#define CHUNKHANNDLER_H
#include <cassert>
#include <string>
#include <Renderer.hpp>
#include <PoolArena.hpp>
#include <Material.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <random>

class ChunkHandler
{

	PoolArena* material_arena;
	MatTexCoords tex_coords;
	int x_chunks;
	int y_chunks;
	int world_width;
	int world_height;


public:

	std::random_device rd;
	std::mt19937 gen;
	struct Move
	{
		vector2 old_pos;
		vector2 new_pos;
	};

	struct Animation
	{
		Material* material;
		float color_change;
		int frames;
	};

	struct Chunk
	{
		std::vector<Material*> materials;
		std::unordered_set<Material*> update_list;
		vector2 d_upper{-1,-1};
		vector2 d_lower{-1, -1};
		vector2 coords {0, 0};
		int asleep;
		int num_materials {0};
	};

	std::vector<Move> move_list;
	std::vector<Move> swap_list;
	std::vector<Material*> trash_materials;
	std::vector<Chunk*> add_materials_list;
	std::vector<Chunk*> delete_chunks;
	std::vector<Chunk*> iter_chunks;
	std::vector<Animation> animation_list;
	std::unordered_map<vector2, Chunk*, vector_hash> chunks;
	int chunk_width;
	int chunk_height;
	int chunk_size;
	const float g_force = 2;
	const int fast_liquid_spread = 5;

	vector2 get_rxn_coord(Material* material);
	std::string print_pos(int x, int y);
	inline size_t index(int x, int y)
	{
		return ((y % chunk_height) * chunk_width) + (x % chunk_width);
	};
	inline Material* get_material(int x, int y)
	{
		vector2 chunk_coords {x/chunk_width, y/chunk_height};

		if(chunks.find(chunk_coords) == chunks.end())
		{
			return nullptr;
		}


		return chunks[chunk_coords]->materials[index(x, y)];
	}
	;

	void set_material_properties(Material* material, MatType type, vector2* pos);
	void acid_mat_rxn(Chunk* chunk, Material* water, Material* other);
	void water_sand_rxn(Chunk* chunk, Material* water, Material* sand);
	void smoke_falling_rxn(Chunk* chunk, Material* smoke, Material* other);
	void smoke_mat_rxn(Chunk* chunk, Material* smoke, Material* other);
	void fire_water_rxn(Chunk* chunk, Material* fire, Material* water);
	void destroy_material(Material* material);
	void make_dirty_rect(Chunk* chunk);
	void modify_rect(Chunk* chunk, vector2* old_pos, vector2* new_pos);
	void commit_changes();
	void init_chunk_handler(int chunk_width, int chunk_height, int world_width,  int world_height, PoolArena* arena);
	void add_materials(const std::vector<Material*>& materials);
	void update_chunk(Chunk* chunk, const float dT);
	void draw_chunk(Chunk* chunk, Renderer* render);
	bool can_react(Material* m1, Material* m2);
	void react(Chunk* chunk, Material* m1, Material* m2);
	void move_material(Chunk* chunk, Material* material, vector2* old_pos, vector2* new_pos);
	void swap_material(Chunk* chunk, Material* material, vector2* old_pos, vector2* new_pos);
	void remove_from_anim_list(Material* material);

	inline Chunk* get_chunk(int x, int y){
		vector2 chunk_coords {x/chunk_width, y/chunk_height};
		if(chunks.find(chunk_coords) == chunks.end())
		{
			return nullptr;
		}
		return chunks[chunk_coords];
	};
	Chunk* init_chunk(int mat_x, int mat_y);

	inline bool in_world(int x, int y){
		return x >= 0 && x < world_width && y >= 0 && y < world_height;
	};
	bool update_short_lived(Material* material);
	bool update_down(Chunk* chunk, Material* material, const float dT);
	bool update_side(Chunk* chunk, Material* material);
	bool update_side_down(Chunk* chunk, Material* material);
	bool update_up(Chunk* chunk, Material* material);
	bool update_side_up(Chunk* chunk, Material* material);
	bool in_anim_list(Material* material);
	~ChunkHandler();

};
#endif
