#include <PoolArena.hpp>
#include <ChunkHandler.hpp>
#include <Renderer.hpp>
#include <Material.hpp>
#include <iostream>

class World
{
public:
	ChunkHandler handler;
	Renderer render;
	MatTexCoords tex_coords;
	int world_width;
	int world_height;
	PoolArena* arena;
	std::vector<serialized_material> save_materials;


	void save_world(std::string name);
	void load_world(std::string name);

	inline void spawn_materials(const vector2& spawn_center, const vector2&draw_size, std::vector<Material*>& materials, MatType spawn_type)
	{		
		int num_cols = draw_size.x;
		int num_rows = draw_size.y;

		int half_x = draw_size.x/2;
		int half_y = draw_size.y/2;

		int lX = spawn_center.x - half_x;
		int rX = spawn_center.x + half_x;
		int tY = spawn_center.y - half_y;
		int bY = spawn_center.y + half_y;

		if(lX < 0)
		{
			num_cols += lX;
			lX = 0;
		}
		if(rX > world_width)

		{
			num_cols -= rX - world_width;
			rX = world_width - 1;
		}
		if(tY < 0)
		{
			num_rows += tY;
			tY = 0;
		}
		if(bY > world_height)
		{
			num_rows -= bY - world_height;
			bY = world_height - 1;
		}

		if(spawn_type == FIRE){
			flame_brush(num_rows * num_cols, rX, bY, lX, tY, materials); 
		}
		else{
			regular_brush(num_rows * num_cols, rX, bY, lX, spawn_type, materials);
		}

		std::cout << "Adding materials at center: " << handler.print_pos(spawn_center.x, spawn_center.y) << "\n";

		handler.add_materials(materials); 
	}

	void create_materials(const vector2& old_center, const vector2& new_center, const vector2& draw_size, const MatType type);
	void delete_materials(int center_x, int center_y, int width, int height);
	void init_world(int chunk_width, int chunk_height, int world_width, int world_height, PoolArena* material_arena);
	void update_world(const float dT);
	void draw_world(bool debug_mode);
	void flame_brush(const int num_materials, int lower_x, int lower_y, int upper_x, int upper_y, std::vector<Material*>& materials);
	void regular_brush(const int num_materials, int lower_x, int lower_y, int upper_x, MatType type, std::vector<Material*>& materials);
};
