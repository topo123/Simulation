#include <iostream>
#include <ChunkHandler.hpp>
#include <cstring>
#include <cstdint>
#include <cassert>


void ChunkHandler::destroy_material(Material* material)
{
	if(material == nullptr)
	{
		return;
	}		
	vector2 pos{material->position.x, material->position.y};
	Chunk* chunk = get_chunk(pos.x, pos.y);
	if(chunk->asleep == 1)
	{
		chunk->asleep = 0;
	}
	assert(chunk != nullptr);
	chunk->update_list.erase(material);
	assert(chunk->materials[index(pos.x, pos.y)] != nullptr);
	chunk->materials[index(pos.x, pos.y)] = nullptr;
	chunk->num_materials --;
	int free_failure = deallocate(material_arena, material);
	assert(free_failure != -1);
}

std::string ChunkHandler::print_pos(int x, int y)
{
	return "(" + std::to_string(x) + ", " + std::to_string(y) + ")"; 
}

ChunkHandler::Chunk* ChunkHandler::init_chunk(int mat_x, int mat_y)
{
	Chunk* new_chunk = new Chunk();
	assert(new_chunk != nullptr);
	new_chunk->asleep = 0;
	new_chunk->coords.x = mat_x/chunk_width;
	new_chunk->coords.y = mat_y/chunk_height;
	new_chunk->materials.resize(chunk_width * chunk_height);
	assert(new_chunk->materials.size() == chunk_width * chunk_height);
	new_chunk->num_materials = 0;
	iter_chunks.push_back(new_chunk);
	chunks[new_chunk->coords] = new_chunk;
	return new_chunk;
}

bool ChunkHandler::in_world(int x, int y)
{
	return x >= 0 && x < world_width && y >= 0 && y < world_height;
}

Material* ChunkHandler::get_material(int x, int y)
{
	
	vector2 chunk_coords {x/chunk_width, y/chunk_height};

	if(chunks.find(chunk_coords) == chunks.end())
	{
		return nullptr;
	}


	assert(chunks.find(chunk_coords) != chunks.end());
	Chunk* chunk = chunks[chunk_coords];
	Material* material = chunk->materials[index(x, y)];

	return material;
}

bool ChunkHandler::can_react(Material* m1, Material* m2)
{
	switch (m1->material)
	{
		case MatType::SAND:
			return m2->material == MatType::WATER || m2->material == ACID;
			break;
		case MatType::ACID:
			return m2->material == WATER || m2->material == STONE || m2->material == SAND;
			break;
		case MatType::WATER:
			return m2->material == ACID;
			break;
		case MatType::SMOKE:
			return m2->property & Properties::DOWN;
			break;
	}
	return false;
}


std::vector<vector2> ChunkHandler::get_rxn_coords(Material* material)
{
	std::vector<vector2> vec_coords;

	if(material->property & Properties::DOWN && material->property & Properties::DOWN_SIDE)
	{
		bool diag_left = in_world(material->position.x - 1, material->position.y + 1) && get_material(material->position.x - 1, material->position.y + 1) != nullptr && can_react(material, get_material(material->position.x - 1, material->position.y + 1));
		bool diag_right = in_world(material->position.x + 1, material->position.y + 1) && get_material(material->position.x + 1, material->position.y + 1) != nullptr && can_react(material, get_material(material->position.x + 1, material->position.y + 1));
		bool down = in_world(material->position.x, material->position.y + 1) && get_material(material->position.x, material->position.y + 1) != nullptr && can_react(material, get_material(material->position.x, material->position.y + 1));
		;
		if(diag_left)
		{
			vec_coords.push_back({material->position.x - 1, material->position.y + 1});
		}
		else if(down)
		{
			vec_coords.push_back({material->position.x, material->position.y + 1});
		}
		else if(diag_right)
		{
			vec_coords.push_back({material->position.x + 1, material->position.y + 1});
		}
	}

	if(material->property & Properties::UP && material->property & Properties::UP_SIDE)
	{
		bool diag_left = in_world(material->position.x - 1, material->position.y - 1) && get_material(material->position.x - 1, material->position.y - 1) != nullptr && can_react(material, get_material(material->position.x - 1, material->position.y - 1));
		bool diag_right = in_world(material->position.x + 1, material->position.y - 1) && get_material(material->position.x + 1, material->position.y - 1) != nullptr && can_react(material, get_material(material->position.x + 1, material->position.y - 1));
		bool up = in_world(material->position.x, material->position.y - 1) && get_material(material->position.x, material->position.y - 1) != nullptr && can_react(material, get_material(material->position.x, material->position.y - 1));
		;
		if(diag_left)
		{
			vec_coords.push_back({material->position.x - 1, material->position.y - 1});
		}
		else if(up)
		{
			vec_coords.push_back({material->position.x, material->position.y - 1});
		}
		else if(diag_right)
		{
			vec_coords.push_back({material->position.x + 1, material->position.y - 1});
		}
	
	}

	if(material->property & Properties::SIDE)
	{
		bool left = in_world(material->position.x - 1, material->position.y) && get_material(material->position.x - 1, material->position.y) != nullptr && can_react(material, get_material(material->position.x - 1, material->position.y));
		bool right = in_world(material->position.x + 1, material->position.y) && get_material(material->position.x + 1, material->position.y) != nullptr && can_react(material, get_material(material->position.x + 1, material->position.y));
		if(left)
		{
			vec_coords.push_back({material->position.x - 1, material->position.y});
		}
		else if(right)
		{
			vec_coords.push_back({material->position.x + 1, material->position.y});
		}
	}
	return vec_coords;
}

ChunkHandler::Chunk* ChunkHandler::move_material(Chunk* chunk, Material* material, bool swap, vector2* old_pos, vector2* new_pos)
{
	if(get_material(new_pos->x, new_pos->y) != nullptr && !swap)
	{
		return nullptr;
	}

	if(get_material(new_pos->x, new_pos->y) == nullptr && swap)
	{
		return nullptr;
	}

	assert(material->position.x == old_pos->x && material->position.y == old_pos->y);
	vector2 new_coords {new_pos->x/chunk_width, new_pos->y/chunk_height};

	if(chunks.find(new_coords) == chunks.end())
	{
		Chunk* new_chunk = init_chunk(new_pos->x, new_pos->y);
		assert(chunk->materials[index(old_pos->x, old_pos->y)] != nullptr);
		chunk->materials[index(old_pos->x, old_pos->y)] = nullptr;
		chunk->num_materials --;
		assert(*chunk->update_list.find(material) == material);
		chunk->update_list.erase(material);
		new_chunk->materials[index(new_pos->x, new_pos->y)] = material;
		new_chunk->update_list.insert(material);
		new_chunk->num_materials ++;
		material->position.x = new_pos->x;
		material->position.y = new_pos->y;
		return new_chunk;
	}
	else if(chunks[new_coords] == chunk)
	{
		if(swap)
		{
			assert(chunk->materials[index(new_pos->x, new_pos->y)] != nullptr);
			assert(chunk->materials[index(old_pos->x, old_pos->y)] == material);
			Material* swap_material = material;
			chunk->materials[index(old_pos->x, old_pos->y)] = chunk->materials[index(new_pos->x, new_pos->y)];
			chunk->materials[index(new_pos->x, new_pos->y)] = swap_material;
			chunk->materials[index(old_pos->x, old_pos->y)]->position.x = old_pos->x;
			chunk->materials[index(old_pos->x, old_pos->y)]->position.y = old_pos->y;
		}
		else
	{
			assert(chunk->materials[index(old_pos->x, old_pos->y)] != nullptr);
			assert(chunk->materials[index(new_pos->x, new_pos->y)] == nullptr);
			chunk->materials[index(new_pos->x, new_pos->y)] = material;
			chunk->materials[index(old_pos->x, old_pos->y)] = nullptr;
		}
		material->position.x = new_pos->x;
		material->position.y = new_pos->y;
		return chunk;
	}
	else if(chunks[new_coords] != chunk)
	{
		
		Chunk* new_chunk = chunks[new_coords];

		if(new_chunk->asleep == 1)
		{
			new_chunk->asleep = 0;
		}
		

		if(swap)
		{
			Material* swap_material = material;
			Material* new_material = new_chunk->materials[index(new_pos->x, new_pos->y)];
			assert(new_material != nullptr);
			assert(chunk->materials[index(old_pos->x, old_pos->y)] == material);
			chunk->materials[index(old_pos->x, old_pos->y)] = new_material;
			new_chunk->materials[index(new_pos->x, new_pos->y)] = swap_material;
			chunk->update_list.erase(swap_material);
			chunk->update_list.insert(new_material);
			new_chunk->update_list.erase(new_material);
			new_chunk->update_list.insert(swap_material);
			new_material->position.x = old_pos->x;
			new_material->position.y = old_pos->y;
		}
		else
	{
			assert(chunk->materials[index(old_pos->x, old_pos->y)] != nullptr);
			assert(new_chunk->materials[index(new_pos->x, new_pos->y)] == nullptr);
			chunk->materials[index(old_pos->x, old_pos->y)] = nullptr;
			assert(*chunk->update_list.find(material) == material);
			chunk->update_list.erase(material);
			chunk->num_materials --;
			new_chunk->materials[index(new_pos->x, new_pos->y)] = material;
			new_chunk->update_list.insert(material);
			new_chunk->num_materials ++;
		}
		material->position.x = new_pos->x;
		material->position.y = new_pos->y;
		return new_chunk;
	}
	else 
{
		assert(false);
	}

	return nullptr;
}

bool ChunkHandler::in_rect(Chunk* chunk, vector2* pos)
{
	return pos->x >= chunk->d_upper.x && pos->x <= chunk->d_lower.x && pos->y >= chunk->d_upper.y && pos->y <= chunk->d_lower.y;
}

void ChunkHandler::expand_rect(Chunk* chunk, vector2* old_pos, vector2* new_pos)
{
	int upper_x, upper_y, lower_x, lower_y;

	upper_x = old_pos->x < new_pos->x? old_pos->x: new_pos->x;
	upper_y = old_pos->y < new_pos->y? old_pos->y: new_pos->y;
	lower_x = old_pos->x > new_pos->x? old_pos->x + 1: new_pos->x + 1;
	lower_y = old_pos->y > new_pos->y? old_pos->y + 1: new_pos->y + 1;

	vector2 upper{upper_x, upper_y};
	vector2 lower{lower_x, lower_y};

	Chunk* lower_chunk = get_chunk(lower.x, lower.y);
	Chunk* upper_chunk = get_chunk(upper.x, upper.y);

	if(chunk == upper_chunk)
	{
		chunk->d_upper.x = upper.x < chunk->d_upper.x || chunk->d_upper.x == -1? upper.x: chunk->d_upper.x;
		chunk->d_upper.y = upper.y < chunk->d_upper.y || chunk->d_upper.y == -1? upper.y: chunk->d_upper.y;
		chunk->d_lower.x = lower.x > chunk->d_lower.x || chunk->d_lower.x == -1? lower.x: chunk->d_lower.x;
		chunk->d_lower.y = lower.y > chunk->d_lower.y || chunk->d_lower.y == -1? lower.y: chunk->d_lower.y;
	}


	if(chunk != upper_chunk && chunk == lower_chunk)
	{
		vector2 new_chunk_coords{upper.x/chunk_width, upper.y/chunk_height};

		if(new_chunk_coords.x == chunk->coords.x && new_chunk_coords.y < chunk->coords.y)
		{
			chunk->d_upper.y = (chunk->coords.y * chunk_height);
			chunk->d_upper.x = upper.x < chunk->d_upper.x || chunk->d_upper.x == -1? upper.x: chunk->d_upper.x;
		}
		else if(new_chunk_coords.x < chunk->coords.x && new_chunk_coords.y == chunk->coords.y)
		{
			chunk->d_upper.x = (chunk->coords.x * chunk_width);
			chunk->d_upper.y = upper.y < chunk->d_upper.y || chunk->d_upper.y == -1? upper.y: chunk->d_upper.y;
		}
		else if(new_chunk_coords.x < chunk->coords.x && new_chunk_coords.y < chunk->coords.y)
		{
			chunk->d_upper.x = chunk->coords.x * chunk_width;
			chunk->d_upper.y = chunk->coords.y * chunk_height;
		}
	}
	if(chunk != lower_chunk && chunk != upper_chunk)
	{
		vector2 lower_chunk_coords {lower.x/chunk_width, lower.y/chunk_height};
		vector2 upper_chunk_coords {upper.x/chunk_width, upper.y/chunk_height};

		if(upper_chunk_coords.x < chunk->coords.x && lower_chunk_coords.y > chunk->coords.y)
		{
			chunk->d_upper.x = chunk->coords.x * chunk_width;
			chunk->d_lower.y = (chunk->coords.y * chunk_height) + chunk_height - 1;
		}
		if(upper_chunk_coords.y > chunk->coords.y && lower_chunk_coords.x > chunk->coords.x)
		{
			chunk->d_upper.y = chunk->coords.y * chunk_height;
			chunk->d_lower.x = (chunk->coords.x * chunk_width) + chunk_width - 1;
		}
	}
	if(chunk != lower_chunk && chunk == upper_chunk)
	{

		vector2 new_chunk_coords{lower.x/chunk_width, lower.y/chunk_height};

		if(new_chunk_coords.x == chunk->coords.x && new_chunk_coords.y > chunk->coords.y)
		{
			chunk->d_lower.y = (chunk->coords.y * chunk_height) + chunk_height - 1;
			chunk->d_lower.x = lower.x > chunk->d_lower.x || chunk->d_lower.x == -1? lower.x: chunk->d_lower.x;
		}
		else if(new_chunk_coords.x > chunk->coords.x && new_chunk_coords.y == chunk->coords.y)
		{
			chunk->d_lower.x = (chunk->coords.x * chunk_width) + chunk_width - 1;
			chunk->d_lower.y = lower.y > chunk->d_lower.y || chunk->d_lower.y == -1? lower.y: chunk->d_lower.y;
		}
		else if(new_chunk_coords.x > chunk->coords.x && new_chunk_coords.y > chunk->coords.y)
		{
			chunk->d_lower.x = (chunk->coords.x * chunk_width) + chunk_width - 1;
			chunk->d_lower.y = (chunk->coords.y * chunk_height) + chunk_height - 1;
		}

	}
	if(chunk->d_lower.x == chunk->d_upper.x && chunk->d_lower.y == chunk->d_upper.y)
	{
		chunk->d_upper.x --;
		chunk->d_upper.y --;
	}

	chunk->d_lower.x = chunk->d_lower.x < 0? lower.x: chunk->d_lower.x;
	chunk->d_lower.y = chunk->d_lower.y < 0? lower.y: chunk->d_lower.y;
	chunk->d_upper.x = chunk->d_upper.x < 0? upper.x: chunk->d_upper.x;
	chunk->d_upper.y = chunk->d_upper.y < 0? upper.y: chunk->d_upper.y;

	assert(chunk->d_upper.x > -1 && chunk->d_upper.y > -1 && chunk->d_lower.x > -1 && chunk->d_lower.y > -1);
}

void ChunkHandler::make_dirty_rect(Chunk* chunk)
{

	for(auto i = chunk->update_list.begin(); i != chunk->update_list.end(); i ++)
	{
		Material* mat = (*i);

		if(mat->property & Properties::STATIC)
		{
			continue;
		}

		assert(mat != nullptr);
		std::vector<vector2> rxn_coords = get_rxn_coords(mat);
		bool up = in_world(mat->position.x, mat->position.y - 1) && get_material(mat->position.x, mat->position.y - 1) == nullptr;
		bool down = in_world(mat->position.x, mat->position.y + 1) && get_material(mat->position.x, mat->position.y + 1) == nullptr;
		bool side_left = in_world(mat->position.x - 1, mat->position.y) && get_material(mat->position.x - 1, mat->position.y) == nullptr;
		bool side_right = in_world(mat->position.x + 1, mat->position.y) && get_material(mat->position.x + 1, mat->position.y) == nullptr;
		bool diag_right = in_world(mat->position.x + 1, mat->position.y + 1) && get_material(mat->position.x + 1, mat->position.y + 1) == nullptr && side_right;
		bool diag_left = in_world(mat->position.x - 1, mat->position.y + 1) && get_material(mat->position.x - 1, mat->position.y + 1) == nullptr && side_left;
		bool up_diag_left = in_world(mat->position.x - 1, mat->position.y - 1) && get_material(mat->position.x - 1, mat->position.y - 1) == nullptr && side_left;
		bool up_diag_right = in_world(mat->position.x + 1, mat->position.y - 1) && get_material(mat->position.x + 1, mat->position.y - 1) == nullptr && side_right;

		if(mat->property & Properties::DOWN && down)
		{
			vector2 new_pos {mat->position.x, mat->position.y + 1};
			expand_rect(chunk, &mat->position, &new_pos);
		}
		else if(mat->property & Properties::UP && up)
		{
			vector2 new_pos{mat->position.x, mat->position.y - 1};
			expand_rect(chunk, &mat->position, &new_pos);
		}
		else if(mat->property & Properties::DOWN_SIDE && diag_right && diag_left)
		{
			vector2 left {mat->position.x - 1, mat->position.y + 1};
			vector2 right {mat->position.x + 1, mat->position.y + 1};
			expand_rect(chunk, &mat->position, &left);
			expand_rect(chunk, &mat->position, &right);
		}
		else if(mat->property & Properties::DOWN_SIDE && diag_right)
		{
			vector2 right {mat->position.x + 1, mat->position.y + 1};
			expand_rect(chunk, &mat->position, &right);
		}
		else if(mat->property & Properties::DOWN_SIDE && diag_left)
		{
			vector2 left {mat->position.x - 1, mat->position.y + 1};
			expand_rect(chunk, &mat->position, &left);
		}
		else if(mat->property & Properties::UP_SIDE && up_diag_left && up_diag_right)
		{
			vector2 left {mat->position.x - 1, mat->position.y - 1};
			vector2 right {mat->position.x + 1, mat->position.y - 1};
			expand_rect(chunk, &mat->position, &left);
			expand_rect(chunk, &mat->position, &right);
		}
		else if(mat->property & Properties::UP_SIDE && up_diag_right)
		{
			vector2 right {mat->position.x + 1, mat->position.y - 1};
			expand_rect(chunk, &mat->position, &right);
		}
		else if(mat->property & Properties::UP_SIDE && up_diag_left)
		{
			vector2 left {mat->position.x - 1, mat->position.y - 1};
			expand_rect(chunk, &mat->position, &left);
		}
		else if(mat->property & Properties::SIDE && side_left && side_right)
		{
			vector2 left {mat->position.x - 1, mat->position.y};
			vector2 right {mat->position.x + 1, mat->position.y};
			expand_rect(chunk, &mat->position, &left);
			expand_rect(chunk, &mat->position, &right);
		}
		else if(mat->property & Properties::SIDE && side_right)
		{
			vector2 right {mat->position.x + 1, mat->position.y};
			expand_rect(chunk, &mat->position, &right);
			dirty.log("Dirty rect upper coords: " + print_pos(chunk->d_upper.x, chunk->d_upper.y) + " Dirty rect lower coords: " + print_pos(chunk->d_lower.x, chunk->d_lower.y));
		}
		else if(mat->property & Properties::SIDE && side_left)
		{
			vector2 left {mat->position.x - 1, mat->position.y};
			expand_rect(chunk, &mat->position, &left);
		}
		else if(!rxn_coords.empty())
		{
			for(size_t i = 0; i < rxn_coords.size(); i ++)
			{
				expand_rect(chunk, &mat->position, &rxn_coords[i]);
			}
		}
	}
}

void ChunkHandler::water_sand_rxn(Chunk* chunk, Material* water, Material* sand)
{
	Move sand_move{sand->position, water->position, true};
	chunk->moves.push_back(sand_move);
}

void ChunkHandler::acid_mat_rxn(Chunk* chunk, Material* acid, Material* other)
{
	other->health -= 10;
	if(other->health <= 0)
	{
		destroy_material(other);
	}
}

void ChunkHandler::smoke_falling_rxn(Chunk* chunk, Material* smoke, Material* other)
{
	Move smoke_move{smoke->position, other->position, true};
	chunk->moves.push_back(smoke_move);
}

void ChunkHandler::react(Chunk* chunk, Material* m1, Material* m2)
{
	if(m1->material == MatType::SAND && (m2->material == MatType::WATER || m2->material == MatType::ACID))
	{
		water_sand_rxn(chunk, m2, m1);
	}
	if(m1->material == ACID)
	{
		acid_mat_rxn(chunk, m1, m2);
	}
	if(m1->material == SMOKE && m2->property & Properties::DOWN)
	{
		smoke_falling_rxn(chunk, m1, m2);
	}

}


bool ChunkHandler::update_down(Chunk* chunk, Material* material)
{
	Material* check_material = get_material(material->position.x, material->position.y + 1);


	if(check_material == nullptr && in_world(material->position.x, material->position.y + 1))
	{
		vector2 new_pos {material->position.x, material->position.y + 1};
		Move new_move {material->position, new_pos, false};
		chunk->moves.push_back(new_move);
		return true;
	}
	return false;
}


bool ChunkHandler::update_up(Chunk* chunk, Material* material)
{
	if(!in_world(material->position.x, material->position.y - 1))
	{
		return false;
	}
	Material* check_material = get_material(material->position.x, material->position.y - 1);


	if(check_material == nullptr && in_world(material->position.x, material->position.y - 1))
	{
		vector2 new_pos {material->position.x, material->position.y - 1};
		Move new_move {material->position, new_pos, false};
		chunk->moves.push_back(new_move);
		return true;
	}
	return false;
}


bool ChunkHandler::update_side_up(Chunk* chunk, Material* material)
{
	uint8_t direction = rand() % 2;
	bool can_move = false;

	if(direction == 0)
	{
		bool left_clear = in_world(material->position.x - 1, material->position.y) && get_material(material->position.x - 1, material->position.y) == nullptr;
		bool diag_left_clear = in_world(material->position.x - 1, material->position.y - 1) && get_material(material->position.x - 1, material->position.y - 1) == nullptr;
		can_move = left_clear && diag_left_clear;
	}
	else
{
		bool right_clear = in_world(material->position.x + 1, material->position.y) && get_material(material->position.x + 1, material->position.y) == nullptr;
		bool diag_right_clear = in_world(material->position.x + 1, material->position.y - 1) && get_material(material->position.x + 1, material->position.y - 1) == nullptr;
		can_move = right_clear && diag_right_clear;
	}

	if(!can_move && direction == 0)
	{
		bool right_clear = in_world(material->position.x + 1, material->position.y) && get_material(material->position.x + 1, material->position.y) == nullptr;
		bool diag_right_clear = in_world(material->position.x + 1, material->position.y - 1) && get_material(material->position.x + 1, material->position.y - 1) == nullptr;
		can_move = right_clear && diag_right_clear;
		direction = 1;
	}
	else if(!can_move && direction == 1)
	{
		bool left_clear = in_world(material->position.x - 1, material->position.y) && get_material(material->position.x - 1, material->position.y) == nullptr;
		bool diag_left_clear = in_world(material->position.x - 1, material->position.y - 1) && get_material(material->position.x - 1, material->position.y - 1) == nullptr;
		can_move = left_clear && diag_left_clear;
		direction = 0;
	}

	if(!can_move)
	{
		return false;
	}

	if(direction == 0)
	{
		vector2 new_pos {material->position.x - 1, material->position.y - 1};
		Move new_move {material->position, new_pos, false};
		chunk->moves.push_back(new_move);
		return true;
	}
	else if(direction == 1)
	{
		vector2 new_pos {material->position.x + 1, material->position.y - 1};
		Move new_move {material->position, new_pos, false};
		chunk->moves.push_back(new_move);
		return true;
	}
	return false;
}


bool ChunkHandler::update_side(Chunk* chunk, Material* material)
{
	bool left = material->velocity.x < 0;
	bool right = material->velocity.x > 0; 
	if(!left || !right)
	{
		uint8_t direction = rand() % 2;
		left = direction == 0;
		right = direction == 1;
	}

	if(left && get_material(material->position.x - 1, material->position.y) == nullptr)
	{
		vector2 new_pos {material->position.x - 1, material->position.y};
		Move move {material->position, new_pos, false};
		assert(move.old_pos == material->position);
		chunk->moves.push_back(move);
		return true;
	}
	if(right && get_material(material->position.x + 1, material->position.y) == nullptr)
	{
		vector2 new_pos {material->position.x + 1, material->position.y};
		Move move {material->position, new_pos, false};
		chunk->moves.push_back(move);
		return true;
	}

	if(left && get_material(material->position.x - 1, material->position.y) != nullptr && get_material(material->position.x + 1, material->position.y) == nullptr)
	{
		vector2 new_pos {material->position.x + 1, material->position.y};
		Move move {material->position, new_pos, false};
		chunk->moves.push_back(move);
		return true;
	}
	if(right && get_material(material->position.x + 1, material->position.y) != nullptr && get_material(material->position.x - 1, material->position.y) == nullptr)
	{
		vector2 new_pos {material->position.x - 1, material->position.y};
		Move move {material->position, new_pos, false};
		chunk->moves.push_back(move);
		return true;
	}
	return false;
}

bool ChunkHandler::update_side_down(Chunk* chunk, Material* material)
{
	uint8_t direction = rand() % 2;
	bool can_move = false;

	if(direction == 0)
	{
		bool left_clear = get_material(material->position.x - 1, material->position.y) == nullptr;
		bool diag_left_clear = get_material(material->position.x - 1, material->position.y + 1) == nullptr;
		can_move = left_clear && diag_left_clear && in_world(material->position.x - 1, material->position.y + 1);
	}
	else
{
		bool right_clear = get_material(material->position.x + 1, material->position.y) == nullptr;
		bool diag_right_clear = get_material(material->position.x + 1, material->position.y + 1) == nullptr;
		can_move = right_clear && diag_right_clear && in_world(material->position.x + 1, material->position.y + 1);
	}

	if(!can_move && direction == 0)
	{
		bool right_clear = get_material(material->position.x + 1, material->position.y) == nullptr;
		bool diag_right_clear = get_material(material->position.x + 1, material->position.y + 1) == nullptr;
		can_move = right_clear && diag_right_clear && in_world(material->position.x + 1, material->position.y + 1);
		direction = 1;
	}
	else if(!can_move && direction == 1)
	{
		bool left_clear = get_material(material->position.x - 1, material->position.y) == nullptr;
		bool diag_left_clear = get_material(material->position.x - 1, material->position.y + 1) == nullptr;
		can_move = left_clear && diag_left_clear && in_world(material->position.x - 1, material->position.y + 1);
		direction = 0;
	}

	if(!can_move)
	{
		return false;
	}

	if(direction == 0)
	{
		vector2 new_pos {material->position.x - 1, material->position.y + 1};
		Move new_move {material->position, new_pos, false};
		chunk->moves.push_back(new_move);
		return true;
	}
	else if(direction == 1)
	{
		vector2 new_pos {material->position.x + 1, material->position.y + 1};
		Move new_move {material->position, new_pos, false};
		chunk->moves.push_back(new_move);
		return true;
	}
	return false;
}

void ChunkHandler::update_chunk(Chunk* chunk)
{
	assert(chunks.find(chunk->coords) != chunks.end());
	std::vector<Material*>& particles = chunk->materials;

	if(chunk->asleep == 1)
	{
		return;
	}




	make_dirty_rect(chunk);

	if(chunk->d_upper.x == -1)
	{
		chunk->asleep = 1;
		return;
	}

	int begin = index(chunk->d_upper.x, chunk->d_upper.y), end = index(chunk->d_lower.x, chunk->d_lower.y) + 1;
	if(begin < 0 || end < 0)
	{
		return;
	}
	if(begin >= 4800)
	{
		std::cout << "Bad\n";
	}
	for(size_t i = begin; i < end; i ++)
	{
		Material* mat = particles[i];
		if(mat == nullptr || mat->property & Properties::STATIC)
		{
			continue;
		}

		bool moved = false;
		if(mat->property & Properties::DOWN && !moved)
		{
			moved = update_down(chunk, mat);
		}
		if(mat->property & Properties::DOWN_SIDE && !moved)
		{
			moved = update_side_down(chunk, mat);
		}
		if(mat->property & Properties::UP && !moved)
		{
			moved = update_up(chunk, mat);
		}
		if(mat->property & Properties::UP_SIDE && !moved)
		{
			moved = update_side_up(chunk, mat);
		}
		if(mat->property & Properties::SIDE && !moved)
		{
			moved = update_side(chunk, mat);
		}

		std::vector<vector2> rxn_coords = get_rxn_coords(mat);
		if(!moved && !rxn_coords.empty())
		{
			int rand_index = rand() % rxn_coords.size();
			Material* react_mat = get_material(rxn_coords[rand_index].x, rxn_coords[rand_index].y);
			react(chunk, mat, react_mat);
		}

	}
}

ChunkHandler::Chunk* ChunkHandler::get_chunk(int x, int y)
{
	vector2 chunk_coords {x/chunk_width, y/chunk_height};
	if(chunks.find(chunk_coords) == chunks.end())
	{
		return nullptr;
	}
	return chunks[chunk_coords];
}



void ChunkHandler::init_chunk_handler(int cW, int cH, int wW, int wH, PoolArena* arena)
{
	if(wW % cW != 0 || wH % cH != 0)
	{
		assert(false);
		return;
	}
	material_arena = arena;

	chunk_width = cW;
	chunk_height = cH;

	world_width = wW;
	world_height = wH;

	x_chunks = wW/cW;
	y_chunks = wH/cH;
	chunk_size = cW * cH;
	logger.init_logger("error.txt");
}

void ChunkHandler::add_materials(const std::vector<Material*>& material)
{
	for(size_t i = 0; i < material.size(); i ++)
	{
		Material* mat = material[i];
		int x = mat->position.x;
		int y = mat->position.y;

		vector2 chunk_pos {x/chunk_width, y/chunk_height};

		if(chunks.find(chunk_pos) == chunks.end())
		{
			Chunk* chunk = init_chunk(x, y);
			assert(chunk != nullptr);
			chunk->update_list.insert(mat);
			chunk->materials[index(x, y)] = mat;
			chunk->num_materials ++;
		}
		else if(chunks[chunk_pos]->materials[index(x, y)] == nullptr)
		{
			chunks[chunk_pos]->asleep = chunks[chunk_pos]->asleep == 1? 0:0;
			chunks[chunk_pos]->update_list.insert(mat);
			chunks[chunk_pos]->materials[index(x, y)] = mat;
			chunks[chunk_pos]->num_materials ++;
		}
		else
	{
			int success = deallocate(material_arena, mat);
			if(success == -1)
			{
				assert(false);
			}
		}
	}
}


void ChunkHandler::commit_changes(Chunk* chunk)
{
	if(chunk->num_materials == 0)
	{
		return;
	}

	std::vector<Move> iter_moves = chunk->moves;

	for(size_t i = 0; i < iter_moves.size(); i ++)
	{
		int rand_index = rand() % iter_moves.size();

		Move move = iter_moves[rand_index];
		Material* material = get_material(move.old_pos.x, move.old_pos.y);
		//assert(material != nullptr);
		if(material != nullptr && material->position.x == move.old_pos.x && material->position.y == move.old_pos.y)
		{
			if(move.swap && get_material(move.new_pos.x, move.new_pos.y) != nullptr)
			{
				move_material(chunk, material, move.swap, &move.old_pos, &move.new_pos);
			}
			else if(!move.swap && get_material(move.new_pos.x, move.new_pos.y) == nullptr)
			{
				move_material(chunk, material, move.swap, &move.old_pos, &move.new_pos);
			}
		}
		iter_moves.erase(iter_moves.begin() + rand_index);
	}
	assert(chunk->update_list.size() == chunk->num_materials);
	chunk->moves.clear();
}

void ChunkHandler::draw_chunk(Chunk* chunk, Renderer render)
{
	const std::vector<Material*>& particles = chunk->materials;
	vector2 end_coords;
	end_coords.x = (chunk->coords.x * chunk_width) + chunk_width;
	end_coords.y = (chunk->coords.y * chunk_height) + chunk_height;
	vector2 start_coords {chunk->coords.x * chunk_width, chunk->coords.y * chunk_height};
	render.draw_rect(start_coords, end_coords, tex_coords.GRID_DEBUG);
	render.draw_rect(chunk->d_upper, chunk->d_lower, tex_coords.DIRTY_DEBUG);
	vector2 mat_size {1, 1};
	for(size_t i = 0; i < chunk_size; i ++)
	{
		Material* mat = particles[i];
		if(particles[i] != nullptr)
		{
			render.render(mat->tex_offset, &mat->position, &mat_size);
		}
	}
	chunk->d_lower.x = -1;
	chunk->d_lower.y = -1;
	chunk->d_upper.x = -1;
	chunk->d_upper.y = -1;
}

size_t ChunkHandler::index(int x, int y)
{
	size_t i = ((y % chunk_height) * chunk_width) + (x % chunk_width);
	//assert(i >= 0 && i < chunk_width * chunk_height);
	return ((y % chunk_height) * chunk_width) + (x % chunk_width);
};
