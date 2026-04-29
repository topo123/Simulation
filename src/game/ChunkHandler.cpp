#include "Material.hpp"
#include "PoolArena.hpp"
#include <iostream>
#include <algorithm>
#include <climits>
#include <ChunkHandler.hpp>
#include <cstring>
#include <cassert>

#define TEX_COORD(MATERIAL) tex_coords.##MATERIAL

void ChunkHandler::remove_from_anim_list(Material* material)
{
	animation_list.erase(material);
}

void ChunkHandler::debug_mat(Material* mat)
{
	std::cout << "Postion: " << print_pos(mat->position.x, mat->position.y) << "; ";
	std::cout << "Velocity: " << element_updater.print_vel(mat->velocity.x, mat->velocity.y) << "; ";
	std::cout << "Rest counter at " << mat->frames_rest << "; ";

	if(mat->phys_state == RESTING)
	{
		std::cout << "Physics state: RESTING\n";
	}
	else if(mat->phys_state == FREE_FALLING)
	{
		std::cout << "Physics state: FREE_FALLING\n";
	}
	else if(mat->phys_state == SLIDING)
	{
		std::cout << "Physics state: SLIDING\n";
	}

}


bool ChunkHandler::in_anim_list(Material* material){
	return animation_list.find(material) != animation_list.end();
}

void ChunkHandler::init_material_props()
{
	material_props.resize(11);
	material_prop_arena = init_pool(11, sizeof(MaterialProps));

	material_props[SAND] = (MaterialProps*)(allocate(material_prop_arena));
	material_props[WATER] = (MaterialProps*)(allocate(material_prop_arena));
	material_props[SMOKE] = (MaterialProps*)(allocate(material_prop_arena));
	material_props[FIRE] = (MaterialProps*)(allocate(material_prop_arena));
	material_props[WOOD] = (MaterialProps*)(allocate(material_prop_arena));
	material_props[STONE] = (MaterialProps*)(allocate(material_prop_arena));
	material_props[ACID] = (MaterialProps*)(allocate(material_prop_arena));
	material_props[NITRO] = (MaterialProps*)(allocate(material_prop_arena));
	material_props[OIL] = (MaterialProps*)(allocate(material_prop_arena));
	material_props[FLAMMABLE_GAS] = (MaterialProps*)(allocate(material_prop_arena));

	*material_props[SAND] = {static_cast<Properties>(PHYSICS), 
		static_cast<ReactionProperties>(ACID_DESTROY),
		static_cast<ReactionDirection>(RDOWN),
		200,
		tex_coords.SAND,
		SOLID
	};

	*material_props[WATER] = {static_cast<Properties>(PHYSICS), 
		static_cast<ReactionProperties>(DISPLACIBLE + ACID_DESTROY),
		static_cast<ReactionDirection>(RNONE),
		100,
		tex_coords.WATER,
		LIQUID
	};

	*material_props[SMOKE] = {static_cast<Properties>(UP + UP_SIDE + SHORT_LIVED), 
		static_cast<ReactionProperties>(NONE),
		static_cast<ReactionDirection>(RUP),
		200,
		tex_coords.SMOKE,
		GAS
	};

	*material_props[FIRE] = {static_cast<Properties>(STATIC + SHORT_LIVED), 
		static_cast<ReactionProperties>(NONE),
		static_cast<ReactionDirection>(RNONE),
		200,
		tex_coords.FIRE,
		ENERGY
	};

	*material_props[WOOD] = {static_cast<Properties>(STATIC), 
		static_cast<ReactionProperties>(FLAMMABLE + ACID_DESTROY),
		static_cast<ReactionDirection>(RNONE),
		500,
		tex_coords.WOOD,
		SOLID
	};

	*material_props[STONE] = {static_cast<Properties>(STATIC), 
		static_cast<ReactionProperties>(ACID_DESTROY),
		static_cast<ReactionDirection>(RNONE),
		800,
		tex_coords.STONE,
		SOLID
	};

	*material_props[ACID] = {static_cast<Properties>(PHYSICS), 
		static_cast<ReactionProperties>(DISPLACIBLE),
		static_cast<ReactionDirection>(AROUND),
		200,
		tex_coords.ACID,
		LIQUID
	};

	/*
	*material_props[NITRO] = {static_cast<Properties>(DOWN_SIDE + DOWN + SIDE), 
		static_cast<ReactionProperties>(NONE),
		static_cast<ReactionDirection>(RNONE),
		200, 
		tex_coords.ACID};
		TODO: Implement an NITRO material that actually works
	*/ 

	*material_props[OIL] = {static_cast<Properties>(PHYSICS), 
		static_cast<ReactionProperties>(FLAMMABLE + DISPLACIBLE + ACID_DESTROY),
		static_cast<ReactionDirection>(AROUND),
		500,
		tex_coords.OIL,
		LIQUID
	};

	*material_props[FLAMMABLE_GAS] = {static_cast<Properties>(UP + UP_SIDE), 
		static_cast<ReactionProperties>(FLAMMABLE),
		static_cast<ReactionDirection>(AROUND),
		200,
		tex_coords.FL_GAS,
		GAS
	};

}

void ChunkHandler::set_material_properties(Material* material, MatType type, vector2* pos)
{
	assert(in_world(pos->x, pos->y));
	if(!material) return;

	material->state = NORMAL;
	material->frames_rest = 0;
	material->phys_state = FREE_FALLING;
	material->material = type;
	material->position.x = pos->x;
	material->position.y = pos->y;
	material->velocity.x = 0;
	material->velocity.y = 1;
	material->health = material_props[type]->health;
	material->tex_offset = material_props[type]->tex_offset;
}


void ChunkHandler::destroy_material(Material* material)
{
	if(material == nullptr)
	{
		return;
	}		
	vector2 pos{material->position.x, material->position.y};
	Chunk* chunk = get_chunk(pos.x, pos.y);
	assert(chunk != nullptr);
	assert(chunk->grid[index(pos.x, pos.y)] != nullptr);
	chunk->grid[index(pos.x, pos.y)] = nullptr;
	chunk->num_materials --;
	int free_failure = deallocate(material_arena, material);
	assert(free_failure != -1);
}

std::string ChunkHandler::print_pos(int x, int y)
{
	return "(" + std::to_string(x) + ", " + std::to_string(y) + ")"; 
}

std::string ChunkHandler::print_pos(vector2& pos)
{
	return "(" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ")"; 
}

std::string ChunkHandler::print_pos(fvector2& pos)
{
	return "(" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ")"; 
}

Chunk* ChunkHandler::init_chunk(int mat_x, int mat_y)
{
	Chunk* new_chunk = new Chunk();

	assert(new_chunk != nullptr);
	new_chunk->lower_rect_coords.x = INT_MIN, new_chunk->lower_rect_coords.y = INT_MIN;
	new_chunk->upper_rect_coords.x = INT_MAX, new_chunk->upper_rect_coords.y = INT_MAX;
	new_chunk->coords.x = mat_x/chunk_width;
	new_chunk->coords.y = mat_y/chunk_height;
	new_chunk->grid = new Material*[chunk_width * chunk_height];

	for(size_t i = 0; i < chunk_size; i ++)
	{
		new_chunk->grid[i] = nullptr;
	}

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
			return material_props[m1->material]->reaction & DISPLACIBLE;
			break;
		case MatType::ACID:
			return material_props[m2->material]->reaction & ACID_DESTROY;
			break;
		case MatType::SMOKE:
			return material_props[m2->material]->property & Properties::DOWN;
			break;
		case MatType::FIRE:
			return material_props[m2->material]->reaction & FLAMMABLE;
			break;
		case MatType::FLAMMABLE_GAS:
			return material_props[m2->material]->property & Properties::DOWN;
			break;
	}
	return false;
}


std::vector<vector2> ChunkHandler::get_rxn_coord(Material* material)
{
	std::vector<vector2> possible_reactions;

	if(material_props[material->material]->reaction_direction == RNONE){
		return possible_reactions;
	}

	vector2 mat_pos{material->position.x, material->position.y};

	if(material_props[material->material]->reaction_direction == RDOWN)
	{
		Material* down = in_world(mat_pos.x, mat_pos.y + 1)? get_material(material->position.x, material->position.y + 1): nullptr;
		Material* down_left = in_world(mat_pos.x - 1, mat_pos.y + 1)? get_material(material->position.x - 1, material->position.y + 1): nullptr;
		Material* down_right = in_world(mat_pos.x + 1, mat_pos.y + 1)? get_material(material->position.x + 1, material->position.y + 1): nullptr;
		if(down != nullptr && can_react(material, down))
		{
			possible_reactions.push_back(down->position);
		}
		if(down_left != nullptr && can_react(material, down_left))
		{
			possible_reactions.push_back(down_left->position);
		}
		if(down_right != nullptr && can_react(material, down_right))
		{
			possible_reactions.push_back(down_right->position);
		}
	}


	if(material_props[material->material]->reaction_direction == RUP)
	{
		Material* up = in_world(mat_pos.x, mat_pos.y - 1)? get_material(material->position.x, material->position.y - 1): nullptr;
		Material* up_left = in_world(mat_pos.x - 1, mat_pos.y - 1)? get_material(material->position.x - 1, material->position.y - 1): nullptr;
		Material* up_right = in_world(mat_pos.x + 1, mat_pos.y - 1)? get_material(material->position.x + 1, material->position.y - 1): nullptr;
		if(up != nullptr && can_react(material, up))
		{
			possible_reactions.push_back(up->position);
		}
		if(up_left != nullptr && can_react(material, up_left))
		{
			possible_reactions.push_back(up_left->position);
		}
		if(up_right != nullptr && can_react(material, up_right))
		{
			possible_reactions.push_back(up_right->position);
		}
	}


	if(material_props[material->material]->reaction_direction == RSIDE)
	{
		Material* left = in_world(mat_pos.x - 1, mat_pos.y)? get_material(material->position.x - 1, material->position.y): nullptr;
		Material* right = in_world(mat_pos.x + 1, mat_pos.y)? get_material(material->position.x + 1, material->position.y): nullptr;
		if(left != nullptr && can_react(material, left))
		{
			possible_reactions.push_back({left->position.x, left->position.y});
		}
		if(right != nullptr && can_react(material, right))
		{
			possible_reactions.push_back({right->position.x, right->position.y});
		}
	}

	if(material_props[material->material]->reaction_direction== AROUND || material->state == BURNING)
	{
		Material* up = in_world(mat_pos.x, mat_pos.y - 1)? get_material(material->position.x, material->position.y - 1): nullptr;
		Material* down = in_world(mat_pos.x, mat_pos.y + 1)? get_material(material->position.x, material->position.y + 1): nullptr;
		Material* up_left = in_world(mat_pos.x - 1, mat_pos.y - 1)? get_material(material->position.x - 1, material->position.y - 1): nullptr;
		Material* up_right = in_world(mat_pos.x + 1, mat_pos.y - 1)? get_material(material->position.x + 1, material->position.y - 1): nullptr;
		Material* left = in_world(mat_pos.x - 1, mat_pos.y)? get_material(material->position.x - 1, material->position.y): nullptr;
		Material* right = in_world(mat_pos.x + 1, mat_pos.y)? get_material(material->position.x + 1, material->position.y): nullptr;
		Material* down_left = in_world(mat_pos.x - 1, mat_pos.y + 1)? get_material(material->position.x - 1, material->position.y + 1): nullptr;
		Material* down_right = in_world(mat_pos.x + 1, mat_pos.y + 1)? get_material(material->position.x + 1, material->position.y + 1): nullptr;
		if(up != nullptr && can_react(material, up))
		{
			possible_reactions.push_back(up->position);
		}
		if(down != nullptr && can_react(material, down))
		{
			possible_reactions.push_back(down->position);
		}
		if(up_left != nullptr && can_react(material, up_left))
		{
			possible_reactions.push_back(up_left->position);
		}
		if(up_right != nullptr && can_react(material, up_right))
		{
			possible_reactions.push_back(up_right->position);
		}
		if(down_right != nullptr && can_react(material, down_right))
		{
			possible_reactions.push_back(down_right->position);
		}
		if(down_left != nullptr && can_react(material, down_left))
		{
			possible_reactions.push_back(down_left->position);
		}
		if(left != nullptr && can_react(material, left))
		{
			possible_reactions.push_back(left->position);
		}
		if(right != nullptr && can_react(material, right))
		{
			possible_reactions.push_back(right->position);
		}
	}

	return possible_reactions;
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
		assert(chunk->grid[index(old_pos->x, old_pos->y)] != nullptr);
		chunk->grid[index(old_pos->x, old_pos->y)] = nullptr;
		chunk->num_materials --;
		assert(chunk->active_list[material->chunk_index] == material);
		//remove_mat_from_update_list(chunk, material);
		new_chunk->grid[index(new_pos->x, new_pos->y)] = material;
		new_chunk->active_list.push_back(material);
		new_chunk->num_materials ++;
		material->position.x = new_pos->x;
		material->position.y = new_pos->y;
		material->chunk_index = 0;
	}
	else if(chunks[new_coords] == chunk)
	{
		assert(chunk->active_list[material->chunk_index] == material);
		assert(chunk->grid[index(old_pos->x, old_pos->y)] != nullptr);
		assert(chunk->grid[index(new_pos->x, new_pos->y)] == nullptr);
		chunk->grid[index(new_pos->x, new_pos->y)] = material;
		chunk->grid[index(old_pos->x, old_pos->y)] = nullptr;
		material->position.x = new_pos->x;
		material->position.y = new_pos->y;
	}
	else if(chunks[new_coords] != chunk)
	{

		Chunk* new_chunk = chunks[new_coords];

		/*
		if(new_chunk->asleep == 1)
		{
			wake_up_materials(chunk);
			new_chunk->asleep = 0;
		}
		*/

		assert(chunk->grid[index(old_pos->x, old_pos->y)] != nullptr);
		assert(new_chunk->grid[index(new_pos->x, new_pos->y)] == nullptr);
		chunk->grid[index(old_pos->x, old_pos->y)] = nullptr;
		assert(chunk->active_list[material->chunk_index] == material);
		//remove_mat_from_update_list(chunk, material);
		chunk->num_materials --;
		new_chunk->grid[index(new_pos->x, new_pos->y)] = material;
		new_chunk->active_list.push_back(material);
		material->chunk_index = new_chunk->num_materials;
		new_chunk->num_materials ++;
		material->position.x = new_pos->x;
		material->position.y = new_pos->y;
	}

}


void ChunkHandler::swap_material(Chunk* chunk, Material* material, vector2* old_pos, vector2* new_pos)
{
	if(get_material(new_pos->x, new_pos->y) == nullptr || material == nullptr || !can_react(material, get_material(new_pos->x, new_pos->y)))
	{
		return;
	}

	vector2 new_coords {new_pos->x/chunk_width, new_pos->y/chunk_height};
	if(chunks[new_coords] == chunk)
	{


		assert(chunk->grid[index(new_pos->x, new_pos->y)] != nullptr);
		assert(chunk->grid[index(old_pos->x, old_pos->y)] == material);
		Material* swap_material = material;
		chunk->grid[index(old_pos->x, old_pos->y)] = chunk->grid[index(new_pos->x, new_pos->y)];
		chunk->grid[index(new_pos->x, new_pos->y)] = swap_material;
		chunk->grid[index(old_pos->x, old_pos->y)]->position.x = old_pos->x;
		chunk->grid[index(old_pos->x, old_pos->y)]->position.y = old_pos->y;

		material->position.x = new_pos->x;
		material->position.y = new_pos->y;
	}
	else if(chunks[new_coords] != chunk)
	{
		Chunk* new_chunk = chunks[new_coords];
		/*
		if(new_chunk->asleep == 1)
		{
			wake_up_materials(chunk);
			new_chunk->asleep = 0;
		}
		*/

		Material* swap_material = material;
		Material* new_material = new_chunk->grid[index(new_pos->x, new_pos->y)];
		assert(chunk->grid[index(old_pos->x, old_pos->y)] == material);
		chunk->grid[index(old_pos->x, old_pos->y)] = new_material;
		new_chunk->grid[index(new_pos->x, new_pos->y)] = swap_material;
		//remove_mat_from_update_list(chunk, swap_material);
		//remove_mat_from_update_list(chunk, new_material);
		chunk->active_list.push_back(new_material);
		new_chunk->active_list.push_back(swap_material);
		new_material->position.x = old_pos->x;
		new_material->position.y = old_pos->y;

		material->position.x = new_pos->x;
		material->position.y = new_pos->y;
	}
}


void ChunkHandler::modify_rect(Chunk* chunk, vector2* new_rect_upper, vector2* new_rect_lower)
{
	vector2* old_rect_upper = &chunk->upper_rect_coords;
	vector2* old_rect_lower = &chunk->lower_rect_coords;

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

	if(chunk->upper_rect_coords.x == INT_MAX)
	{
		chunk->upper_rect_coords.x = new_rect_upper->x;
		chunk->upper_rect_coords.y = new_rect_upper->y;
		chunk->lower_rect_coords.x = new_rect_lower->x;
		chunk->lower_rect_coords.y = new_rect_lower->y;
		return;
	}

	if(new_rect_upper->x > chunk->upper_rect_coords.x && new_rect_lower->x < chunk->lower_rect_coords.x && new_rect_upper->y > chunk->upper_rect_coords.y && new_rect_lower->y < chunk->lower_rect_coords.y)
	{
		chunk->upper_rect_coords.x = new_rect_upper->x;
		chunk->upper_rect_coords.y = new_rect_upper->y;
		chunk->lower_rect_coords.x = new_rect_lower->x;
		chunk->lower_rect_coords.y = new_rect_lower->y;
		return;
	}

	if(!(new_rect_upper->x > chunk->lower_rect_coords.x || new_rect_upper->y > chunk->lower_rect_coords.y || new_rect_lower->x < chunk->upper_rect_coords.x || new_rect_lower->y < chunk->upper_rect_coords.y))
	{
		chunk->upper_rect_coords.x = chunk->upper_rect_coords.x > new_rect_upper->x? new_rect_upper->x: chunk->upper_rect_coords.x;
		chunk->upper_rect_coords.y = chunk->upper_rect_coords.y > new_rect_upper->y? new_rect_upper->y: chunk->upper_rect_coords.y;
		chunk->lower_rect_coords.x = chunk->lower_rect_coords.x < new_rect_lower->x? new_rect_lower->x: chunk->lower_rect_coords.x;
		chunk->lower_rect_coords.y = chunk->lower_rect_coords.y < new_rect_lower->y? new_rect_lower->y: chunk->lower_rect_coords.y;
	}
	else
{
		chunk->upper_rect_coords.x = new_rect_upper->x;
		chunk->upper_rect_coords.y = new_rect_upper->y;
		chunk->lower_rect_coords.x = new_rect_lower->x;
		chunk->lower_rect_coords.y = new_rect_lower->y;
	}

	//assert(chunk->upper_rect_coords.x < chunk->lower_rect_coords.x && chunk->upper_rect_coords.y < chunk->lower_rect_coords.y);
	//assert(chunk->upper_rect_coords.x > 0 && chunk->upper_rect_coords.y > 0);


}

void ChunkHandler::make_dirty_rect(Chunk* chunk)
{

	if(chunk->active_list.size() == 0)
	{
		chunk->upper_rect_coords.x = -1;
		chunk->upper_rect_coords.y = -1;
		chunk->lower_rect_coords.x = -1;
		chunk->lower_rect_coords.x = -1;
		return;
	}

	int min_x = INT_MAX;
	int min_y = INT_MAX;
	int max_x = INT_MIN;
	int max_y = INT_MIN;

	const std::vector<Material*>& mat_list = chunk->active_list;
	std::vector<PhysicsMaterialMoveData>& move_data = chunk->move_data;

	for(size_t i = 0; i < mat_list.size(); i ++)
	{
		Material* mat = mat_list[i];

		int mat_x = mat->position.x;
		int mat_y = mat->position.y;

		if(mat->state == BURNING)
		{
			min_x = std::min(mat_x, min_x);
			min_y = std::min(mat_y, min_y);
			max_y = std::max(mat_y, max_y);
			max_x = std::max(mat_x, max_x);
			continue;
		}
		if(material_props[mat->material]->property & Properties::SHORT_LIVED)
		{
			min_x = std::min(mat_x, min_x);
			min_y = std::min(mat_y, min_y);
			max_y = std::max(mat_y, max_y);
			max_x = std::max(mat_x, max_x);
			continue;
		}
		if(material_props[mat->material]->property & Properties::PHYSICS)
		{
			Material* hit_mat = nullptr;
			vector2 update_pos = element_updater.pos_update(&mat->position, &mat->velocity, &hit_mat);

			if(update_pos.x != -1)
			{
				move_data.emplace_back();
				move_data.back().org_material = mat;
				move_data.back().hit_material = hit_mat;
				move_data.back().new_pos = update_pos;

				min_x = std::min(mat_x, min_x);
				min_y = std::min(mat_y, min_y);

				max_x = std::max(mat_x, max_x);
				max_y = std::max(mat_y, max_y);

				min_x = std::min(update_pos.x, min_x);
				min_y = std::min(update_pos.y, min_y);

				max_x = std::max(update_pos.x, max_x);
				max_y = std::max(update_pos.y, max_y);
			}
		}
	}

	vector2 upper_rect {min_x, min_y};
	vector2 lower_rect {max_x, max_y};
	modify_rect(chunk, &upper_rect, &lower_rect); 
}

void ChunkHandler::update_chunk(Chunk* chunk, const float dT)
{

	if(chunk->active_list.size() == 0)
	{
		return;
	}

	const std::vector<PhysicsMaterialMoveData>& move_data = chunk->move_data;
	make_dirty_rect(chunk);

	for(size_t i = 0; i < move_data.size(); i ++)
	{
		const PhysicsMaterialMoveData* phys_move_data = &move_data[i];
		Material* mat = phys_move_data->org_material;

		if(mat->material == SAND)
		{
			element_updater.update_sand(dT, phys_move_data);
		}
		else if(mat->material == WATER)
		{
			element_updater.update_water(dT, phys_move_data);
		}
		else if(mat->material == SMOKE)
		{
			element_updater.update_smoke(dT, phys_move_data);
		}
		else if(mat->material == FIRE)
		{
			element_updater.update_fire(dT, phys_move_data);
		}
		else if(mat->material == WOOD)
		{
			element_updater.update_wood(dT, phys_move_data);
		}
		else if(mat->material == ACID)
		{
			element_updater.update_acid(dT, phys_move_data);
		}
		else if(mat->material == OIL)
		{
			element_updater.update_oil(dT, phys_move_data);
		}
		else if(mat->material == FLAMMABLE_GAS)
		{
			element_updater.update_flammable_gas(dT, phys_move_data);
		}

	}

}



void ChunkHandler::init_chunk_handler(unsigned int x_chunks, unsigned int y_chunks, int wW, int wH, PoolArena* arena)
{
	element_updater.init(this, wW, wH);
	material_arena = arena;

	chunk_width = (wW % x_chunks == 0)? wW/x_chunks: (wW/x_chunks) + 1;
	chunk_height = (wH % y_chunks == 0)? wH/y_chunks: (wH/y_chunks) + 1;

	world_width = wW;
	world_height = wH;

	this->x_chunks = x_chunks;
	this->y_chunks = y_chunks;
	chunk_size = chunk_width * chunk_height;
	gen.seed(rd());

	element_updater.dummy_material = (Material*)allocate(material_arena);
	element_updater.dummy_material->state = NORMAL;
	element_updater.dummy_material->phys_state = RESTING;
	element_updater.dummy_material->material = NONE;
	element_updater.dummy_material->position.x = -1;
	element_updater.dummy_material->position.y = -1;
	element_updater.dummy_material->velocity.x = -1;
	element_updater.dummy_material->velocity.y = -1;
	element_updater.dummy_material->health = -1;
	element_updater.dummy_material->tex_offset = -1.0f;

	init_material_props();
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
			mat->chunk_index = 0;
			assert(chunk != nullptr);
			chunk->active_list.push_back(mat);
			chunk->grid[index(x, y)] = mat;
			chunk->num_materials ++;
		}
		else if(!get_material(x, y))
		{
			mat->chunk_index = chunks[chunk_pos]->num_materials;
			//chunks[chunk_pos]->asleep = 0;
			chunks[chunk_pos]->active_list.push_back(mat);
			assert(chunks[chunk_pos]->active_list[mat->chunk_index] == mat);
			chunks[chunk_pos]->grid[index(x, y)] = mat;
			chunks[chunk_pos]->num_materials ++;
		}
		else
		{
			std::cout << "Deallocating Material	at ";
			std::cout <<  print_pos(x, y) << std::endl;
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

	Animation* animation;
	std::vector<Material*> clear_list;
	for(auto i = animation_list.begin(); i != animation_list.end(); i ++){
		animation = &(i->second);
		i->first->tex_offset = animation->color_change;
		animation->frames --;
		if(animation->frames == 0){
			clear_list.push_back(i->first);
		}
	}

	for(size_t i = 0; i < clear_list.size(); i ++)
	{
		animation_list.erase(clear_list[i]);
	}

	swap_list.clear();
}

void ChunkHandler::draw_chunks_to_texture(Renderer* render, bool debug_mode)
{
	std::vector<float> tex_offsets;
	std::vector<glm::mat4> transforms;
	std::vector<glm::mat4> debug_rect_transforms;
	std::vector<glm::mat4> chunk_rect_transforms;


	static vector2 size {1, 1};

	for(size_t i = 0; i < iter_chunks.size(); i ++)
	{
		Chunk* chunk = iter_chunks[i];

		if(debug_mode)
		{
			glm::mat4 chunk_model = glm::mat4(1.0f);
			glm::mat4 dirty_model = glm::mat4(1.0f);

			vector2 start_coords {chunk->coords.x * chunk_width, chunk->coords.y * chunk_height};
			vector2 end_coords;
			end_coords.x = (chunk->coords.x * chunk_width) + chunk_width;
			end_coords.y = (chunk->coords.y * chunk_height) + chunk_height;

			chunk_model = glm::translate(chunk_model, glm::vec3(start_coords.x, start_coords.y, 0.0f));
			chunk_model = glm::scale(chunk_model, glm::vec3(chunk_width, chunk_height, 1.0f));

			if(chunk->lower_rect_coords.x != - 1)
			{
				dirty_model = glm::translate(dirty_model, glm::vec3(chunk->upper_rect_coords.x, chunk->upper_rect_coords.y, 0.0f));
				dirty_model = glm::scale(dirty_model, glm::vec3((chunk->lower_rect_coords.x - chunk->upper_rect_coords.x), (chunk->lower_rect_coords.y - chunk->upper_rect_coords.y), 1.0f));
			}

			chunk_rect_transforms.push_back(chunk_model);
			debug_rect_transforms.push_back(dirty_model);
		}

		for(size_t j = 0; j < chunk->active_list.size(); j ++)
		{
			tex_offsets.push_back(chunk->active_list[j]->tex_offset);

			glm::mat4 model = glm::mat4(1.0f);

			model = glm::translate(model, glm::vec3(chunk->active_list[j]->position.x, chunk->active_list[j]->position.y, 0.0f));
			model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));
			transforms.push_back(model);

		}
	}

	render->draw_materials(tex_offsets, transforms, size);
	render->draw_empty_rect(debug_rect_transforms, tex_coords.DIRTY_DEBUG);
	render->draw_empty_rect(chunk_rect_transforms, tex_coords.GRID_DEBUG);
}

ChunkHandler::~ChunkHandler()
{
	assert(free_arena(material_arena) == 0);
	move_list.clear();
	swap_list.clear();
	chunks.clear();
	for(int i = 0; i < iter_chunks.size(); i ++)
	{
		delete[] iter_chunks[i]->grid;
		iter_chunks[i]->active_list.clear();
	}
}


