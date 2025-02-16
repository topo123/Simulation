#include <PoolArena.hpp>
#include <ChunkHandler.hpp>
#include <Renderer.hpp>

class World
{
public:
	ChunkHandler handler;
	Renderer render;
	MatTexCoords tex_coords;
	int world_width;
	int world_height;
	PoolArena* arena;

	void create_materials(int center_x, int center_y, int width, int height, MatType type);
	void delete_materials(int center_x, int center_y, int width, int height);
	void init_world(int chunk_width, int chunk_height, int world_width, int world_height, PoolArena* material_arena);
	void update_world();
	void draw_world();
	void set_material_properties(Material* material, MatType type, vector2* pos);
};
