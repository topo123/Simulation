#ifndef CHUNKHANNDLER_H
#define CHUNKHANNDLER_H
#include "Elements.hpp"
#include <cassert>
#include <string>
#include <Renderer.hpp>
#include <PoolArena.hpp>
#include <Material.hpp>
#include <unordered_map>
#include <unordered_set>
#include <array>
#include <vector>
#include <random>

class ChunkHandler
{

	PoolArena* material_arena;
	PoolArena* material_prop_arena;

	int x_chunks;
	int y_chunks;


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
		float color_change;
		int frames;
	};

	int chunk_width;
	int chunk_height;

	struct Chunk
	{
		Material** materials;
		std::vector<Material*> update_list;
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
	std::vector<MaterialProps*> material_props;

	std::unordered_set<Chunk*> set_asleep_already;

	std::unordered_map<Material*, Animation> animation_list;
	std::unordered_map<vector2, Chunk*, vector_hash> chunks;

	MatTexCoords tex_coords;
	Elements element_updater;
	int chunk_size;
	int world_width;
	int world_height;
	const float g_force = 2;
	const int fast_liquid_spread = 5;

	std::string print_pos(int x, int y);
	std::string print_pos(vector2& pos);
	std::string print_pos(fvector2& pos);

	std::vector<vector2> get_rxn_coord(Material* material);

	inline int index(int x, int y)
	{
		return ((y % chunk_height) * chunk_width) + (x % chunk_width);
	};

	inline Material* get_material(int x, int y)
	{
		Chunk* chunk = get_chunk(x, y);
		return chunk != nullptr? chunk->materials[index(x, y)]: nullptr;
	};

	bool in_anim_list(Material* material);
	bool can_react(Material* m1, Material* m2);

	void debug_mat(Material* mat);
	void set_material_properties(Material* material, MatType type, vector2* pos);
	void destroy_material(Material* material);
	void make_dirty_rect(Chunk* chunk);
	void modify_rect(Chunk* chunk, vector2* old_pos, vector2* new_pos);
	void commit_changes();
	void init_chunk_handler(int chunk_width, int chunk_height, int world_width, int world_height, PoolArena* arena);
	void add_materials(const std::vector<Material*>& materials);
	void update_chunk(Chunk* chunk, const float dT);
	void draw_chunk_to_texture(Chunk* chunk, Renderer* render, bool debug_mode);
	void move_material(Chunk* chunk, Material* material, vector2* old_pos, vector2* new_pos);
	void swap_material(Chunk* chunk, Material* material, vector2* old_pos, vector2* new_pos);
	void remove_from_anim_list(Material* material);
	void wake_up_neighbor_chunks(Chunk* chunk);
	void init_material_props();
	

	inline void wake_up_materials(Chunk* chunk)
	{
		for(Material* mat: chunk->update_list)
		{
			mat->phys_state = FREE_FALLING;
		}
	};
	inline Chunk* get_chunk(int x, int y){
		vector2 chunk_coords {x/chunk_width, y/chunk_height};
		if(chunk_coords.x >= x_chunks || chunk_coords.x < 0 || chunk_coords.y >= y_chunks || chunk_coords.y < 0) return nullptr;
		auto it = chunks.find(chunk_coords);
		return it != chunks.end()? it->second: nullptr;
	};
	Chunk* init_chunk(int mat_x, int mat_y);

	inline bool in_world(int x, int y){
		return x >= 0 && x < world_width && y >= 0 && y < world_height;
	};

	inline void remove_mat_from_update_list(Chunk* chunk, Material* remove_material)
	{
		chunk->update_list.back()->chunk_index = remove_material->chunk_index;
		chunk->update_list.back()->chunk_index = remove_material->chunk_index;
		chunk->update_list[remove_material->chunk_index] = chunk->update_list.back();
		chunk->update_list.pop_back();
	}

	~ChunkHandler();

};
#endif
