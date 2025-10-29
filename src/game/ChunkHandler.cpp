#include "Material.hpp"
#include "PoolArena.hpp"
#include <iostream>
#include <algorithm>
#include <climits>
#include <cstdint>
#include <ChunkHandler.hpp>
#include <cstring>
#include <cstdint>
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
	if(chunk->asleep == 1)
	{
		wake_up_materials(chunk);
		chunk->asleep = 0;
	}
	assert(chunk != nullptr);
	remove_mat_from_update_list(chunk, material);
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

std::string ChunkHandler::print_pos(vector2& pos)
{
	return "(" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ")"; 
}

std::string ChunkHandler::print_pos(fvector2& pos)
{
	return "(" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ")"; 
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
	new_chunk->materials = new Material*[chunk_width * chunk_height];

	for(size_t i = 0; i < chunk_size; i ++)
	{
		new_chunk->materials[i] = nullptr;
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
		assert(chunk->materials[index(old_pos->x, old_pos->y)] != nullptr);
		chunk->materials[index(old_pos->x, old_pos->y)] = nullptr;
		chunk->num_materials --;
		assert(chunk->update_list[material->chunk_index] == material);
		remove_mat_from_update_list(chunk, material);
		new_chunk->materials[index(new_pos->x, new_pos->y)] = material;
		new_chunk->update_list.push_back(material);
		new_chunk->num_materials ++;
		material->position.x = new_pos->x;
		material->position.y = new_pos->y;
		material->chunk_index = 0;
	}
	else if(chunks[new_coords] == chunk)
	{
		assert(chunk->update_list[material->chunk_index] == material);
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
			wake_up_materials(chunk);
			new_chunk->asleep = 0;
		}

		assert(chunk->materials[index(old_pos->x, old_pos->y)] != nullptr);
		assert(new_chunk->materials[index(new_pos->x, new_pos->y)] == nullptr);
		chunk->materials[index(old_pos->x, old_pos->y)] = nullptr;
		assert(chunk->update_list[material->chunk_index] == material);
		remove_mat_from_update_list(chunk, material);
		chunk->num_materials --;
		new_chunk->materials[index(new_pos->x, new_pos->y)] = material;
		new_chunk->update_list.push_back(material);
		material->chunk_index = new_chunk->num_materials;
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
			wake_up_materials(chunk);
			new_chunk->asleep = 0;
		}

		Material* swap_material = material;
		Material* new_material = new_chunk->materials[index(new_pos->x, new_pos->y)];
		assert(chunk->materials[index(old_pos->x, old_pos->y)] == material);
		chunk->materials[index(old_pos->x, old_pos->y)] = new_material;
		new_chunk->materials[index(new_pos->x, new_pos->y)] = swap_material;
		remove_mat_from_update_list(chunk, swap_material);
		remove_mat_from_update_list(chunk, new_material);
		chunk->update_list.push_back(new_material);
		new_chunk->update_list.push_back(swap_material);
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
	//assert(chunk->d_upper.x > 0 && chunk->d_upper.y > 0);


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
		if(material_props[mat->material]->property & Properties::STATIC && !(material_props[mat->material]->property & Properties::SHORT_LIVED) && mat->state != BURNING)
		{
			continue;
		}

		int mat_x = mat->position.x;
		int mat_y = mat->position.y;

		if(mat->state == BURNING)
		{
			min_x = std::min(mat_x, min_x);
			min_y = std::min(mat_y, min_y);
			max_y = std::max(mat_y, max_y);
			max_x = std::max(mat_x, max_x);
			is_dirty = 1;
			continue;
		}
		if(material_props[mat->material]->property & Properties::SHORT_LIVED)
		{
			min_x = std::min(mat_x, min_x);
			min_y = std::min(mat_y, min_y);
			max_y = std::max(mat_y, max_y);
			max_x = std::max(mat_x, max_x);
			is_dirty = 1;
			continue;
		}
		if(material_props[mat->material]->property & Properties::PHYSICS)
		{
			Material* hit_mat = nullptr;
			vector2 update_pos = element_updater.pos_update(&mat->position, &mat->velocity, &hit_mat);

			if(update_pos.x != -1)
			{
				min_x = std::min(mat_x, min_x);
				min_y = std::min(mat_y, min_y);

				max_x = std::max(mat_x, max_x);
				max_y = std::max(mat_y, max_y);

				min_x = std::min(update_pos.x, min_x);
				min_y = std::min(update_pos.y, min_y);

				max_x = std::max(update_pos.x, max_x);
				max_y = std::max(update_pos.y, max_y);
				is_dirty = 1;
			}
			else if(mat->phys_state != RESTING)
			{
				min_x = std::min(mat_x, min_x);
				min_y = std::min(mat_y, min_y);

				max_x = std::max(mat_x, max_x);
				max_y = std::max(mat_y, max_y);

				is_dirty = 1;
			}
			else if(material_props[mat->material]->mat_state == LIQUID && mat->phys_state == RESTING)
			{
				Material* left =  in_world(mat_x - 1, mat_y)? get_material(mat_x - 1, mat_y): element_updater.dummy_material;
				Material* right = in_world(mat_x + 1, mat_y)? get_material(mat_x + 1, mat_y): element_updater.dummy_material;

				if(!left)
				{
					min_x = std::min(min_x, mat_x - 1);
					min_y = std::min(min_y, mat_y);

					max_x = std::max(max_x, mat_x);
					max_y = std::max(max_y, mat_y);
					is_dirty = 1;
				}
				if(!right)
				{
					min_x = std::min(min_x, mat_x);
					min_y = std::min(min_y, mat_y);

					max_x = std::max(max_x, mat_x + 1);
					max_y = std::max(max_y, mat_y);
					is_dirty = 1;
				}
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
		chunk->asleep = 0;
	}
	else{
		std::cout << "Chunk " << print_pos(chunk->coords.x, chunk->coords.y) << " is asleep." << std::endl;
		chunk->asleep = 1;
		chunk->d_upper.x = -1;
		chunk->d_upper.y = -1;
		chunk->d_lower.x = -1;
		chunk->d_lower.y = -1;
	}
}
void ChunkHandler::update_chunk(Chunk* chunk, const float dT)
{
	assert(chunks.find(chunk->coords) != chunks.end());

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
	for(size_t i = begin; i < end; i ++)
	{
		Material* mat = chunk->materials[i];
		if(mat == nullptr || material_props[mat->material]->property & Properties::STATIC)
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
		else if(mat->material == OIL)
		{
			element_updater.update_oil(mat, dT);
		}
		else if(mat->material == FLAMMABLE_GAS)
		{
			element_updater.update_flammable_gas(mat, dT);
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
			chunk->update_list.push_back(mat);
			chunk->materials[index(x, y)] = mat;
			chunk->num_materials ++;
		}
		else if(!get_material(x, y))
		{
			mat->chunk_index = chunks[chunk_pos]->num_materials;
			chunks[chunk_pos]->asleep = 0;
			chunks[chunk_pos]->update_list.push_back(mat);
			assert(chunks[chunk_pos]->update_list[mat->chunk_index] == mat);
			chunks[chunk_pos]->materials[index(x, y)] = mat;
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

			dirty_model = glm::translate(chunk_model, glm::vec3(chunk->d_upper.x, chunk->d_upper.y, 0.0f));
			dirty_model = glm::scale(chunk_model, glm::vec3((chunk->d_lower.x - chunk->d_upper.x), (chunk->d_lower.y - chunk->d_upper.y), 1.0f));

			chunk_rect_transforms.push_back(chunk_model);
			debug_rect_transforms.push_back(dirty_model);
		}

		for(size_t j = 0; j < chunk->update_list.size(); j ++)
		{
			tex_offsets.push_back(chunk->update_list[j]->tex_offset);

			glm::mat4 model = glm::mat4(1.0f);

			model = glm::translate(model, glm::vec3(chunk->update_list[j]->position.x, chunk->update_list[j]->position.y, 0.0f));
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
		delete[] iter_chunks[i]->materials;
		iter_chunks[i]->update_list.clear();
	}
}


