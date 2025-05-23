#include <PoolArena.hpp>
#include <ChunkHandler.hpp>
#include <Renderer.hpp>
#include <Material.hpp>

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

	void create_materials(int center_x, int center_y, const vector2& draw_size, MatType type);
	void delete_materials(int center_x, int center_y, int width, int height);
	void init_world(int chunk_width, int chunk_height, int world_width, int world_height, PoolArena* material_arena);
	void update_world(const float dT);
	void draw_world(bool debug_mode);
	void flame_brush(const int num_materials, int lower_x, int lower_y, int upper_x, int upper_y, std::vector<Material*>& materials);
	void regular_brush(const int num_materials, int lower_x, int lower_y, int upper_x, MatType type, std::vector<Material*>& materials);
};
