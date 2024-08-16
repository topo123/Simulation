#include <GameState.hpp>
#include <PoolArena.hpp>
#include <cstdint>
#include <vector>
MatTexCoords tex_coords;
int mat_count = 0;

Material* create_material(MatType material, MaterialState* mat_state, PoolArena* arena, std::vector<std::vector<Material*>>& grid, glm::vec2 pos)
{
	mat_count ++;
	int gX = (int)pos.x;
	int gY = (int)pos.y;
	if(gX < 0)
	{
		gX = 0;
	}
	if(gX > 800)
	{
		gX = 800 - 1;
	}
	if(gY < 0)
	{
		gY = 0;
	}
	if(gY > 600)
	{
		gY = 600 - 1;
	}

	if(grid[gY][gX] != nullptr)
	{
		mat_count --;
		return nullptr;
	}

	assert(gX > -1 && gX < 800);
	assert(gY > -1 && gY < 600);
	Material* mat = static_cast<Material*>(allocate(arena));
	if(mat == nullptr)
	{
		return nullptr;
	}

	mat->material = material;
	mat->velocity = glm::vec2(0.0f, 0.0f);
	mat->position = glm::vec2(gX, gY);

	if(material == MatType::SAND)
	{
		mat->tex_offset = tex_coords.SAND;
		mat->property = static_cast<Properties>(static_cast<uint8_t>(Properties::DOWN_SIDE) + static_cast<uint8_t>(Properties::SIDE));
	}
	else if(material == MatType::WATER)
	{
		mat->tex_offset = tex_coords.WATER;
		mat->property = static_cast<Properties>(static_cast<uint8_t>(Properties::SIDE) + static_cast<uint8_t>(Properties::DOWN));
	}

	mat_state->active_materials.push_back(mat);
	grid[gY][gX] = mat;
	return mat;

}
