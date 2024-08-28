#include <cstdint>
#include <PoolArena.hpp>
#include <ChunkHandler.hpp>
#include <Renderer.hpp>

class World
{
public:
	ChunkHandler handler;
	Renderer render;
	MatTexCoords tex_coords;
	uint16_t world_width;
	uint16_t world_height;

	void create_materials(uint16_t center_x, uint16_t center_y, uint16_t width, uint16_t height, MatType type, PoolArena* arena);
	void init_world(uint16_t chunk_width, uint16_t chunk_height, uint16_t world_width, uint16_t world_height);
	void update_world();
	void draw_world();
	void set_material_properties(Material* material, MatType type, std::pair<uint16_t, uint16_t> pos);
};
