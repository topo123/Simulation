#include "Material.hpp"
#include <iostream>
#include <algorithm>
#include <climits>
#include <cstdint>
#include <ChunkHandler.hpp>
#include <cstring>
#include <cstdint>
#include <cassert>

void ChunkHandler::remove_from_anim_list(Material* material)
{
	for(int i = 0; i < animation_list.size(); i ++)
	{
		if(animation_list[i].material == material)
		{
			animation_list[i] = animation_list.back();
			animation_list.pop_back();
		}
	}
}


bool ChunkHandler::in_anim_list(Material* material){
	for(int i = 0; i < animation_list.size(); i ++)
	{
		if(animation_list[i].material == material)
		{
			return true;
		}
	}
	return false;
}

void ChunkHandler::set_material_properties(Material* material, MatType type, vector2* pos)
{
	material->state = NORMAL;
	material->material = type;
	material->position.x = pos->x;
	material->position.y = pos->y;
	material->velocity.x = 0;
	material->velocity.y = 1;
	if(type == WATER)
	{
		material->health = 100;
		material->tex_offset = tex_coords.WATER;
		material->property = static_cast<Properties>(DOWN_SIDE + DOWN + SIDE);
		material->reaction = static_cast<ReactionProperties>(DISPLACIBLE + ACID_DESTROY);
	}
	else if(type == SAND)
	{
		material->health = 200;
		material->tex_offset = tex_coords.SAND;
		material->property = static_cast<Properties>(DOWN + DOWN_SIDE);
		material->reaction = static_cast<ReactionProperties>(ACID_DESTROY);
	}
	else if(type == STONE)
	{
		material->health = 800;
		material->tex_offset = tex_coords.STONE;
		material->property = static_cast<Properties>(STATIC);
		material->reaction = static_cast<ReactionProperties>(ACID_DESTROY);
	}
	else if(type == ACID)
	{
		material->tex_offset = tex_coords.ACID;
		material->health = 200;
		material->property = static_cast<Properties>(DOWN_SIDE + DOWN + SIDE);
		material->reaction = static_cast<ReactionProperties>(DISPLACIBLE);
	}
	else if(type == SMOKE)
	{
		material->tex_offset = tex_coords.SMOKE;
		material->health = 200;
		material->property = static_cast<Properties>(UP_SIDE + UP);
		material->reaction = static_cast<ReactionProperties>(NONE);
	}
	else if(type == WOOD)
	{
		material->tex_offset = tex_coords.WOOD;
		material->health = 500;
		material->property = static_cast<Properties>(STATIC);
		material->reaction = static_cast<ReactionProperties>(FLAMMABLE + ACID_DESTROY);
	}
	else if(type == FIRE)
	{
		material->tex_offset = tex_coords.FIRE;
		material->health = 200;
		material->property = static_cast<Properties>(STATIC + SHORT_LIVED);
		material->reaction = static_cast<ReactionProperties>(NONE);
	}
}

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
	new_chunk->d_lower.x = INT_MIN, new_chunk->d_lower.y = INT_MIN;
	new_chunk->d_upper.x = INT_MAX, new_chunk->d_upper.y = INT_MAX;
	new_chunk->asleep = 0;
	new_chunk->coords.x = mat_x/chunk_width;
	new_chunk->coords.y = mat_y/chunk_height;
	new_chunk->materials.resize(chunk_width * chunk_height);
	assert(new_chunk->materials.size() == chunk_width * chunk_height);
	new_chunk->num_materials = 0;
	add_materials_list.push_back(new_chunk);
	chunks[new_chunk->coords] = new_chunk;
	return new_chunk;
}


bool ChunkHandler::can_react(Material* m1, Material* m2)
{
	switch (m1->material)
	{
		case MatType::SAND:
			return m2->reaction & DISPLACIBLE;
			break;
		case MatType::ACID:
			return m2->reaction & ACID_DESTROY;
			break;
		case MatType::SMOKE:
			return m2->property & Properties::DOWN;
			break;
		case MatType::FIRE:
			return m2->reaction & FLAMMABLE;
			break;
	}
	return false;
}


std::vector<vector2> ChunkHandler::get_rxn_coord(Material* material)
{
	vector2 mat_pos{material->position.x, material->position.y};
	Material* up = in_world(mat_pos.x, mat_pos.y - 1)? get_material(material->position.x, material->position.y - 1): nullptr;
	Material* down = in_world(mat_pos.x, mat_pos.y + 1)? get_material(material->position.x, material->position.y + 1): nullptr;
	Material* up_left = in_world(mat_pos.x - 1, mat_pos.y - 1)? get_material(material->position.x - 1, material->position.y - 1): nullptr;
	Material* up_right = in_world(mat_pos.x + 1, mat_pos.y - 1)? get_material(material->position.x + 1, material->position.y - 1): nullptr;
	Material* left = in_world(mat_pos.x - 1, mat_pos.y)? get_material(material->position.x - 1, material->position.y): nullptr;
	Material* right = in_world(mat_pos.x + 1, mat_pos.y)? get_material(material->position.x + 1, material->position.y): nullptr;
	Material* down_left = in_world(mat_pos.x - 1, mat_pos.y + 1)? get_material(material->position.x - 1, material->position.y + 1): nullptr;
	Material* down_right = in_world(mat_pos.x + 1, mat_pos.y + 1)? get_material(material->position.x + 1, material->position.y + 1): nullptr;

	std::vector<vector2> react_pos;
	if(up != nullptr && can_react(material, up))
	{
		react_pos.push_back({up->position.x, up->position.y});
	}
	if(down != nullptr && can_react(material, down))
	{
		react_pos.push_back({down->position.x, down->position.y});
	}
	if(up_left != nullptr && can_react(material, up_left))
	{
		react_pos.push_back({up_left->position.x, up_left->position.y});
	}
	if(up_right != nullptr && can_react(material, up_right))
	{
		react_pos.push_back({up_right->position.x, up_right->position.y});
	}
	if(down_right != nullptr && can_react(material, down_right))
	{
		react_pos.push_back({down_right->position.x, down_right->position.y});
	}
	if(down_left != nullptr && can_react(material, down_left))
	{
		react_pos.push_back({down_left->position.x, down_left->position.y});
	}
	if(left != nullptr && can_react(material, left))
	{
		react_pos.push_back({left->position.x, left->position.y});
	}
	if(right != nullptr && can_react(material, right))
	{
		react_pos.push_back({right->position.x, right->position.y});
	}

	return react_pos;
}

void ChunkHandler::move_material(Chunk* chunk, Material* material, vector2* old_pos, vector2* new_pos)
{
	if(get_material(new_pos->x, new_pos->y) != nullptr || material == nullptr)
	{
		return;
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
	}
	else if(chunks[new_coords] == chunk)
	{
		assert(chunk->materials[index(old_pos->x, old_pos->y)] != nullptr);
		assert(chunk->materials[index(new_pos->x, new_pos->y)] == nullptr);
		chunk->materials[index(new_pos->x, new_pos->y)] = material;
		chunk->materials[index(old_pos->x, old_pos->y)] = nullptr;
		material->position.x = new_pos->x;
		material->position.y = new_pos->y;
	}
	else if(chunks[new_coords] != chunk)
	{

		Chunk* new_chunk = chunks[new_coords];

		if(new_chunk->asleep == 1)
		{
			new_chunk->asleep = 0;
		}

		assert(chunk->materials[index(old_pos->x, old_pos->y)] != nullptr);
		assert(new_chunk->materials[index(new_pos->x, new_pos->y)] == nullptr);
		chunk->materials[index(old_pos->x, old_pos->y)] = nullptr;
		assert(*chunk->update_list.find(material) == material);
		chunk->update_list.erase(material);
		chunk->num_materials --;
		new_chunk->materials[index(new_pos->x, new_pos->y)] = material;
		new_chunk->update_list.insert(material);
		new_chunk->num_materials ++;
		material->position.x = new_pos->x;
		material->position.y = new_pos->y;
	}
	else 
{
		assert(false);
	}

}


void ChunkHandler::swap_material(Chunk* chunk, Material* material, vector2* old_pos, vector2* new_pos)
{
	if(get_material(new_pos->x, new_pos->y) == nullptr || material == nullptr || !can_react(material, get_material(new_pos->x, new_pos->y)))
	{
		return;
	}

	assert(chunk->asleep == 0);
	vector2 new_coords {new_pos->x/chunk_width, new_pos->y/chunk_height};
	if(chunks[new_coords] == chunk)
	{


		assert(chunk->materials[index(new_pos->x, new_pos->y)] != nullptr);
		assert(chunk->materials[index(old_pos->x, old_pos->y)] == material);
		Material* swap_material = material;
		chunk->materials[index(old_pos->x, old_pos->y)] = chunk->materials[index(new_pos->x, new_pos->y)];
		chunk->materials[index(new_pos->x, new_pos->y)] = swap_material;
		chunk->materials[index(old_pos->x, old_pos->y)]->position.x = old_pos->x;
		chunk->materials[index(old_pos->x, old_pos->y)]->position.y = old_pos->y;

		material->position.x = new_pos->x;
		material->position.y = new_pos->y;
	}
	else if(chunks[new_coords] != chunk)
	{
		Chunk* new_chunk = chunks[new_coords];
		if(new_chunk->asleep == 1)
		{
			new_chunk->asleep = 0;
		}

		Material* swap_material = material;
		Material* new_material = new_chunk->materials[index(new_pos->x, new_pos->y)];
		assert(chunk->materials[index(old_pos->x, old_pos->y)] == material);
		chunk->materials[index(old_pos->x, old_pos->y)] = new_material;
		new_chunk->materials[index(new_pos->x, new_pos->y)] = swap_material;
		chunk->update_list.erase(swap_material);
		chunk->update_list.insert(new_material);
		new_chunk->update_list.erase(new_material);
		new_chunk->update_list.insert(swap_material);
		new_material->position.x = old_pos->x;
		new_material->position.y = old_pos->y;

		material->position.x = new_pos->x;
		material->position.y = new_pos->y;
	}
}


void ChunkHandler::modify_rect(Chunk* chunk, vector2* new_rect_upper, vector2* new_rect_lower)
{
	vector2* old_rect_upper = &chunk->d_upper;
	vector2* old_rect_lower = &chunk->d_lower;

	vector2 upper_chunk {new_rect_upper->x/chunk_width, new_rect_upper->y/chunk_height};
	vector2 lower_chunk {new_rect_lower->x/chunk_width, new_rect_lower->y/chunk_height};


	if(upper_chunk.x < chunk->coords.x && upper_chunk.y == chunk->coords.y)
	{
		new_rect_upper->x = chunk->coords.x * chunk_width;
	}
	else if(upper_chunk.x == chunk->coords.x && upper_chunk.y < chunk->coords.y)
	{
		new_rect_upper->y = chunk->coords.y * chunk_height;
	}
	else if(upper_chunk.x < chunk->coords.x && upper_chunk.y < chunk->coords.y)
	{
		new_rect_upper->x = chunk->coords.x * chunk_width;
		new_rect_upper->y = chunk->coords.y * chunk_height;
	}

	if(lower_chunk.x > chunk->coords.x && lower_chunk.y == chunk->coords.y)
	{
		new_rect_lower->x = chunk->coords.x * chunk_width + chunk_width - 1;
	}
	else if(lower_chunk.x == chunk->coords.x && lower_chunk.y > chunk->coords.y)
	{
		new_rect_lower->y = chunk->coords.y * chunk_height + chunk_height - 1;
	}
	else if(lower_chunk.x > chunk->coords.x && lower_chunk.y > chunk->coords.y)
	{
		new_rect_lower->x = chunk->coords.x * chunk_width + chunk_width - 1;
		new_rect_lower->y = chunk->coords.y * chunk_height + chunk_height - 1;
	}

	if(chunk->d_upper.x == INT_MAX)
	{
		chunk->d_upper.x = new_rect_upper->x;
		chunk->d_upper.y = new_rect_upper->y;
		chunk->d_lower.x = new_rect_lower->x;
		chunk->d_lower.y = new_rect_lower->y;
		return;
	}

	if(new_rect_upper->x > chunk->d_upper.x && new_rect_lower->x < chunk->d_lower.x && new_rect_upper->y > chunk->d_upper.y && new_rect_lower->y < chunk->d_lower.y)
	{
		chunk->d_upper.x = new_rect_upper->x;
		chunk->d_upper.y = new_rect_upper->y;
		chunk->d_lower.x = new_rect_lower->x;
		chunk->d_lower.y = new_rect_lower->y;
		return;
	}

	if(!(new_rect_upper->x > chunk->d_lower.x || new_rect_upper->y > chunk->d_lower.y || new_rect_lower->x < chunk->d_upper.x || new_rect_lower->y < chunk->d_upper.y))
	{
		chunk->d_upper.x = chunk->d_upper.x > new_rect_upper->x? new_rect_upper->x: chunk->d_upper.x;
		chunk->d_upper.y = chunk->d_upper.y > new_rect_upper->y? new_rect_upper->y: chunk->d_upper.y;
		chunk->d_lower.x = chunk->d_lower.x < new_rect_lower->x? new_rect_lower->x: chunk->d_lower.x;
		chunk->d_lower.y = chunk->d_lower.y < new_rect_lower->y? new_rect_lower->y: chunk->d_lower.y;
	}
	else
{
		chunk->d_upper.x = new_rect_upper->x;
		chunk->d_upper.y = new_rect_upper->y;
		chunk->d_lower.x = new_rect_lower->x;
		chunk->d_lower.y = new_rect_lower->y;
	}

	//assert(chunk->d_upper.x < chunk->d_lower.x && chunk->d_upper.y < chunk->d_lower.y);


}

void ChunkHandler::make_dirty_rect(Chunk* chunk)
{

	int min_x = INT_MAX;
	int min_y = INT_MAX;
	int max_x = INT_MIN;
	int max_y = INT_MIN;
	uint8_t is_dirty = 0;


	for(auto i = chunk->update_list.begin(); i != chunk->update_list.end(); i ++)
	{
		Material* mat = (*i);
		if(mat->property & Properties::STATIC && !(mat->property & Properties::SHORT_LIVED) && mat->state != BURNING)
		{
			continue;
		}


		int mat_x = mat->position.x;
		int mat_y = mat->position.y;
		const int check_side_cells = fast_liquid_spread;
		const int check_down_cells = int(mat->velocity.y + 0.5);

		if(mat->state == BURNING)
		{
			min_x = std::min(mat_x, min_x);
			min_y = std::min(mat_y, min_y);
			max_y = std::max(mat_y, max_y);
			max_x = std::max(mat_x, max_x);
			is_dirty = 1;
			continue;
		}
		if(mat->property & Properties::SHORT_LIVED){
			min_x = std::min(mat_x, min_x);
			min_y = std::min(mat_y, min_y);
			max_y = std::max(mat_y, max_y);
			max_x = std::max(mat_x, max_x);
			is_dirty = 1;
			continue;
		}
		if(mat->property & Properties::DOWN)
		{
			int curr_offset = 1;

			while(in_world(mat_x, mat_y + curr_offset) && get_material(mat_x, mat_y + curr_offset) == nullptr && curr_offset <= check_down_cells){
				curr_offset ++;
			}

			if(curr_offset > 1)
			{
				min_x = std::min(mat_x, min_x);
				min_y = std::min(mat_y, min_y);
				max_y = std::max(mat_y + curr_offset - 1, max_y);
				max_x = std::max(mat_x, max_x);
				is_dirty = 1;
			}
		}
		if(mat->property & Properties::UP && in_world(mat_x, mat_y - 1) && get_material(mat_x, mat_y - 1) == nullptr)
		{
			min_x = std::min(mat_x, min_x);
			min_y = std::min(mat_y - 1, min_y);
			max_y = std::max(mat_y, max_y);
			max_x = std::max(mat_x, max_x);
			assert(min_x <= max_x && min_y <= max_y);
			is_dirty = 1;
		}
		if(mat->property & Properties::DOWN_SIDE && in_world(mat_x + 1, mat_y) && get_material(mat_x + 1, mat_y) == nullptr && in_world(mat_x + 1, mat_y + 1) && get_material(mat_x + 1, mat_y + 1) == nullptr)
		{
			min_x = std::min(mat_x, min_x);
			min_y = std::min(mat_y, min_y);
			max_y = std::max(mat_y + 1, max_y);
			max_x = std::max(mat_x + 1, max_x);
			assert(min_x <= max_x && min_y <= max_y);
			is_dirty = 1;
		}
		if(mat->property & Properties::DOWN_SIDE && in_world(mat_x - 1, mat_y) && get_material(mat_x - 1, mat_y) == nullptr && in_world(mat_x - 1, mat_y + 1) && get_material(mat_x - 1, mat_y + 1) == nullptr)
		{
			min_x = std::min(mat_x - 1, min_x);
			min_y = std::min(mat_y, min_y);
			max_y = std::max(mat_y + 1, max_y);
			max_x = std::max(mat_x, max_x);
			assert(min_x <= max_x && min_y <= max_y);
			is_dirty = 1;
		}
		if(mat->property & Properties::UP_SIDE && in_world(mat_x + 1, mat_y) && get_material(mat_x + 1, mat_y) == nullptr && in_world(mat_x + 1, mat_y - 1) && get_material(mat_x + 1, mat_y - 1) == nullptr)
		{
			min_x = std::min(mat_x, min_x);
			min_y = std::min(mat_y - 1, min_y);
			max_y = std::max(mat_y,  max_y);
			max_x = std::max(mat_x + 1, max_x);
			assert(min_x <= max_x && min_y <= max_y);
			is_dirty = 1;
		}
		if(mat->property & Properties::UP_SIDE && in_world(mat_x - 1, mat_y) && get_material(mat_x - 1, mat_y) == nullptr && in_world(mat_x - 1, mat_y - 1) && get_material(mat_x - 1, mat_y - 1) == nullptr)
		{
			min_x = std::min(mat_x - 1, min_x);
			min_y = std::min(mat_y - 1, min_y);
			max_y = std::max(mat_y,  max_y);
			max_x = std::max(mat_x, max_x);
			assert(min_x <= max_x && min_y <= max_y);
			is_dirty = 1;
		}
		if(mat->property & Properties::SIDE)
		{
			int curr_offset = 1;
			while(in_world(mat_x - curr_offset, mat_y) && get_material(mat_x - curr_offset, mat_y) == nullptr && curr_offset <= check_side_cells){
				curr_offset ++;
			}

			if(curr_offset > 0)
			{
				min_x = std::min(mat_x - curr_offset + 1, min_x);
				min_y = std::min(mat_y, min_y);
				max_y = std::max(mat_y,  max_y);
				max_x = std::max(mat_x, max_x);
				is_dirty = 1;
			}
		}
		if(mat->property & Properties::SIDE)
		{
			int curr_offset = 1;
			while(in_world(mat_x + curr_offset, mat_y) && get_material(mat_x + curr_offset, mat_y) == nullptr && curr_offset <= check_side_cells){
				curr_offset ++;
			}

			if(curr_offset > 1)
			{
				min_x = std::min(mat_x - curr_offset + 1, min_x);
				min_y = std::min(mat_y, min_y);
				max_y = std::max(mat_y,  max_y);
				max_x = std::max(mat_x, max_x);
				is_dirty = 1;
			}
		}
		else
	{
			std::vector<vector2> rxn_coords = get_rxn_coord(mat);
			if(rxn_coords.size() > 0)
			{
				vector2 rxn_coord = rxn_coords[rand() % rxn_coords.size()];
				min_x = std::min(rxn_coord.x, min_x);
				min_y = std::min(rxn_coord.y, min_y);
				max_y = std::max(rxn_coord.y,  max_y);
				max_x = std::max(rxn_coord.x, max_x);
				is_dirty = 1;
			}
		}

	}

	if(is_dirty)
	{
		vector2 upper_rect {min_x, min_y};
		vector2 lower_rect {max_x, max_y};
		modify_rect(chunk, &upper_rect, &lower_rect); 
	}
	else{
		std::cout << "The chunk " << print_pos(chunk->coords.x, chunk->coords.y) << " is asleep.\n";
		chunk->asleep = 1;
	}
}

void ChunkHandler::fire_water_rxn(Chunk* chunk, Material* fire, Material* water)
{
	fire->health -= 5;
	set_material_properties(water, SMOKE, &water->position);
}
void ChunkHandler::water_sand_rxn(Chunk* chunk, Material* water, Material* sand)
{
	Move sand_move{{sand->position.x, sand->position.y}, {water->position.x, water->position.y}};
	chunk->asleep = 0;
	swap_list.push_back(sand_move);
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
	Move smoke_move{smoke->position, other->position};
	chunk->asleep = 0;
	swap_list.push_back(smoke_move);
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
	if(m1->material == FIRE && m2->material == WATER)
	{
		fire_water_rxn(chunk, m1, m2);
	}
	if(m1->material == WATER && m2->material == FIRE)
	{
		fire_water_rxn(chunk, m2, m1);
	}

}



void ChunkHandler::update_chunk(Chunk* chunk, const float dT)
{
	assert(chunks.find(chunk->coords) != chunks.end());
	std::vector<Material*>& particles = chunk->materials;

	if(chunk->asleep == 1)
	{
		return;
	}

	make_dirty_rect(chunk);

	if(chunk->d_upper.x == INT_MAX)
	{
		return;
	}

	int begin = index(chunk->d_upper.x, chunk->d_upper.y), end = index(chunk->d_lower.x, chunk->d_lower.y) + 1;
	for(size_t i = begin; i < end; i ++)
	{
		Material* mat = particles[i];
		if(mat == nullptr || mat->property & Properties::STATIC)
		{
			continue;
		}

		if(mat->material == SAND)
		{
			element_updater.update_sand(mat, dT);
		}
		else if(mat->material == WATER)
		{
			element_updater.update_water(mat, dT);
		}
		else if(mat->material == SMOKE)
		{
			element_updater.update_smoke(mat, dT);
		}
		else if(mat->material == FIRE)
		{
			element_updater.update_fire(mat, dT);
		}
		else if(mat->material == WOOD)
		{
			element_updater.update_wood(mat, dT);
		}
		else if(mat->material == ACID)
		{
			element_updater.update_acid(mat, dT);
		}


	}
}



void ChunkHandler::init_chunk_handler(int cW, int cH, int wW, int wH, PoolArena* arena)
{

	if(wW % cW != 0 || wH % cH != 0)
	{
		assert(false);
		return;
	}
	element_updater.init(this);
	material_arena = arena;

	chunk_width = cW;
	chunk_height = cH;

	world_width = wW;
	world_height = wH;

	x_chunks = wW/cW;
	y_chunks = wH/cH;
	chunk_size = cW * cH;
	gen.seed(rd());
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
			chunks[chunk_pos]->asleep = chunks[chunk_pos]->asleep == 1? 0: 0;
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


void ChunkHandler::commit_changes()
{

	std::shuffle(move_list.begin(), move_list.end(), gen);
	std::shuffle(swap_list.begin(), swap_list.end(), gen);
	std::shuffle(animation_list.begin(), animation_list.end(), gen);
	Material* material;

	const size_t num_moves = move_list.size()/2;
	const size_t num_swaps = swap_list.size()/2;
	for(size_t i = 0; i < num_moves; i ++)
	{
		Move move = move_list[i];
		material = get_material(move.old_pos.x, move.old_pos.y);
		move_material(get_chunk(move.old_pos.x, move.old_pos.y), material, &move.old_pos, &move.new_pos);
		move_list[i] = move_list.back();
		move_list.pop_back();
	}

	for(size_t i = 0; i < swap_list.size(); i ++)
	{
		Move move = swap_list[i];
		material = get_material(move.old_pos.x, move.old_pos.y);
		swap_material(get_chunk(move.old_pos.x, move.old_pos.y), material, &move.old_pos, &move.new_pos);
	}


	for(size_t i = 0; i < animation_list.size(); i ++){
		animation_list[i].material->tex_offset = animation_list[i].color_change;
		animation_list[i].frames --;
		if(animation_list[i].frames == 0){
			animation_list[i] = animation_list.back();
			animation_list.pop_back();
		}
	}

	swap_list.clear();
}

void ChunkHandler::draw_chunk(Chunk* chunk, Renderer* render)
{
	const std::vector<Material*>& particles = chunk->materials;
	vector2 end_coords;
	end_coords.x = (chunk->coords.x * chunk_width) + chunk_width;
	end_coords.y = (chunk->coords.y * chunk_height) + chunk_height;
	vector2 start_coords {chunk->coords.x * chunk_width, chunk->coords.y * chunk_height};
	render->draw_rect(start_coords, end_coords, tex_coords.GRID_DEBUG);
	render->draw_rect(chunk->d_upper, chunk->d_lower, tex_coords.DIRTY_DEBUG);
	vector2 mat_size {1, 1};
	for(size_t i = 0; i < chunk_size; i ++)
	{
		Material* mat = particles[i];
		if(particles[i] != nullptr)
		{
			render->render(mat->tex_offset, &mat->position, &mat_size);
		}
	}
}

ChunkHandler::~ChunkHandler()
{
	assert(free_arena(material_arena) == 0);
	move_list.clear();
	swap_list.clear();
	chunks.clear();
	for(int i = 0; i < iter_chunks.size(); i ++)
	{
		iter_chunks[i]->materials.clear();
		iter_chunks[i]->update_list.clear();
	}
}


