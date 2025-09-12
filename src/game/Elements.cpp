#include "Elements.hpp"
#include "ChunkHandler.hpp"
#include <cstdint>
#include <iostream>
#include <random>
#include <string>

std::string Elements::print_vel(float x, float y)
{
	return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
}


void Elements::update_sand(ELEMENT_UPDATE_ARGS)
{
	//Try to move
	vector2 updated_pos = solid_physics_update(material, dT, 0.8f, 2.0f);
	if(updated_pos.x != -1)
	{
		handler->move_list.push_back({material->position, updated_pos});
		return;
	}

	//If not move then swap position with whatever below sand
	Material* displace_mat = handler->in_world(material->position.x, material->position.y + 1)? handler->get_material(material->position.x, material->position.y + 1): nullptr;
	Material* left_diag_mat = handler->in_world(material->position.x - 1, material->position.y + 1)? handler->get_material(material->position.x - 1, material->position.y + 1): nullptr;
	Material* right_diag_mat = handler->in_world(material->position.x + 1, material->position.y + 1)? handler->get_material(material->position.x + 1, material->position.y + 1): nullptr;

	if(displace_mat != nullptr && displace_mat->material != SAND && handler->material_props[displace_mat->material]->reaction & DISPLACIBLE)
	{
		handler->swap_list.push_back({material->position, displace_mat->position});
	}
	else if(left_diag_mat != nullptr && left_diag_mat->material != SAND && handler->material_props[left_diag_mat->material]->reaction & DISPLACIBLE && rand() % 2 == 1)
	{
		handler->swap_list.push_back({material->position, left_diag_mat->position});
	}
	else if(right_diag_mat != nullptr && right_diag_mat->material != SAND && handler->material_props[right_diag_mat->material]->reaction & DISPLACIBLE)
	{
		handler->swap_list.push_back({material->position, right_diag_mat->position});
	}
}

void Elements::update_smoke(ELEMENT_UPDATE_ARGS)
{

	Material* displace_mat;
	if(handler->in_world(material->position.x, material->position.y - 1))
	{
		displace_mat = handler->get_material(material->position.x, material->position.y - 1);
	}
	else {
		return;
	}

	if(displace_mat != nullptr && displace_mat->material != SMOKE && handler->material_props[displace_mat->material]->property != STATIC)
	{
		handler->swap_list.push_back({material->position, displace_mat->position});
		return;
	}
	material->health -= 10;
	if(material->health <= 0)
	{
		handler->destroy_material(material);
	}
}


void Elements::update_water(ELEMENT_UPDATE_ARGS)
{

}

void Elements::update_wood(ELEMENT_UPDATE_ARGS)
{
	if(material->state != BURNING)
	{
		return;
	}

	if(rand() % 2 == 1)
	{
		material->health -= 5;
	}

	if(material->health <= 0)
	{
		handler->remove_from_anim_list(material);
		handler->destroy_material(material);
		return;
	}

	auto it = handler->animation_list.find(material);

	if(it == handler->animation_list.end() && material->tex_offset == handler->tex_coords.WOOD && rand () % 10 < 5)
	{
		handler->animation_list[material] = {handler->tex_coords.FIRE, 33};
	}
	else if(it == handler->animation_list.end() && material->tex_offset == handler->tex_coords.FIRE && rand() % 10 < 5)
	{
		handler->animation_list[material] = {handler->tex_coords.FIRE_CHANGE_COLOR, 33};
	}
	else if(it == handler->animation_list.end() && material->tex_offset == handler->tex_coords.FIRE_CHANGE_COLOR && rand() % 10 < 3)
	{
		handler->animation_list[material] = {handler->tex_coords.WOOD, 33};
	}

	vector2 mat_pos{material->position.x, material->position.y};
	Material* up = handler->in_world(mat_pos.x, mat_pos.y - 1)? handler->get_material(material->position.x, material->position.y - 1): nullptr;
	Material* down = handler->in_world(mat_pos.x, mat_pos.y + 1)? handler->get_material(material->position.x, material->position.y + 1): nullptr;
	Material* up_left = handler->in_world(mat_pos.x - 1, mat_pos.y - 1)? handler->get_material(material->position.x - 1, material->position.y - 1): nullptr;
	Material* up_right = handler->in_world(mat_pos.x + 1, mat_pos.y - 1)? handler->get_material(material->position.x + 1, material->position.y - 1): nullptr;
	Material* left = handler->in_world(mat_pos.x - 1, mat_pos.y)? handler->get_material(material->position.x - 1, material->position.y): nullptr;
	Material* right = handler->in_world(mat_pos.x + 1, mat_pos.y)? handler->get_material(material->position.x + 1, material->position.y): nullptr;
	Material* down_left = handler->in_world(mat_pos.x - 1, mat_pos.y + 1)? handler->get_material(material->position.x - 1, material->position.y + 1): nullptr;
	Material* down_right = handler->in_world(mat_pos.x + 1, mat_pos.y + 1)? handler->get_material(material->position.x + 1, material->position.y + 1): nullptr;

	ChunkHandler::Chunk* mat_chunk;
	if(up != nullptr && handler->material_props[up->material]->reaction & FLAMMABLE)
	{
		up->state = BURNING;
		mat_chunk = handler->get_chunk(up->position.x, up->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(down != nullptr && handler->material_props[down->material]->reaction & FLAMMABLE)
	{
		down->state = BURNING;
		mat_chunk = handler->get_chunk(down->position.x, down->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(up_left != nullptr && handler->material_props[up_left->material]->reaction & FLAMMABLE)
	{
		up_left->state = BURNING;
		mat_chunk = handler->get_chunk(up_left->position.x, up_left->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(up_right != nullptr && handler->material_props[up_right->material]->reaction & FLAMMABLE)
	{
		up_right->state = BURNING;
		mat_chunk = handler->get_chunk(up_right->position.x, up_right->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(down_right != nullptr && handler->material_props[down_right->material]->reaction & FLAMMABLE)
	{
		down_right->state = BURNING;
		mat_chunk = handler->get_chunk(down_right->position.x, down_right->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(down_left != nullptr && handler->material_props[down_left->material]->reaction & FLAMMABLE)
	{
		down_left->state = BURNING;
		mat_chunk = handler->get_chunk(down_left->position.x, down_left->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(left != nullptr && handler->material_props[left->material]->reaction & FLAMMABLE)
	{
		left->state = BURNING;
		mat_chunk = handler->get_chunk(left->position.x, left->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(right != nullptr && handler->material_props[right->material]->reaction & FLAMMABLE)
	{
		right->state = BURNING;
		mat_chunk = handler->get_chunk(right->position.x, right->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
}

void Elements::update_fire(ELEMENT_UPDATE_ARGS)
{
	material->health -= 3;

	vector2 mat_pos{material->position.x, material->position.y};
	if(material->health <= 0)
	{
		handler->remove_from_anim_list(material);
		handler->set_material_properties(material, SMOKE, &material->position);
		return;
	}

	auto it = handler->animation_list.find(material);

	if(it == handler->animation_list.end() && material->tex_offset == handler->tex_coords.FIRE)
	{
		handler->animation_list[material] =  {handler->tex_coords.FIRE_CHANGE_COLOR, 33};
	}
	else if(it == handler->animation_list.end() && material->tex_offset == handler->tex_coords.FIRE_CHANGE_COLOR)
	{
		handler->animation_list[material] =  {handler->tex_coords.FIRE, 33};
	}

	Material* up = handler->in_world(mat_pos.x, mat_pos.y - 1)? handler->get_material(material->position.x, material->position.y - 1): nullptr;
	Material* down = handler->in_world(mat_pos.x, mat_pos.y + 1)? handler->get_material(material->position.x, material->position.y + 1): nullptr;
	Material* up_left = handler->in_world(mat_pos.x - 1, mat_pos.y - 1)? handler->get_material(material->position.x - 1, material->position.y - 1): nullptr;
	Material* up_right = handler->in_world(mat_pos.x + 1, mat_pos.y - 1)? handler->get_material(material->position.x + 1, material->position.y - 1): nullptr;
	Material* left = handler->in_world(mat_pos.x - 1, mat_pos.y)? handler->get_material(material->position.x - 1, material->position.y): nullptr;
	Material* right = handler->in_world(mat_pos.x + 1, mat_pos.y)? handler->get_material(material->position.x + 1, material->position.y): nullptr;
	Material* down_left = handler->in_world(mat_pos.x - 1, mat_pos.y + 1)? handler->get_material(material->position.x - 1, material->position.y + 1): nullptr;
	Material* down_right = handler->in_world(mat_pos.x + 1, mat_pos.y + 1)? handler->get_material(material->position.x + 1, material->position.y + 1): nullptr;

	if(up != nullptr && handler->material_props[up->material]->reaction & FLAMMABLE)
	{
		up->state = BURNING;
	}
	if(down != nullptr && handler->material_props[down->material]->reaction & FLAMMABLE)
	{
		down->state = BURNING;
	}
	if(up_left != nullptr && handler->material_props[up_left->material]->reaction & FLAMMABLE)
	{
		up_left->state = BURNING;
	}
	if(up_right != nullptr && handler->material_props[up_right->material]->reaction & FLAMMABLE)
	{
		up_right->state = BURNING;
	}
	if(down_right != nullptr && handler->material_props[down_right->material]->reaction & FLAMMABLE)
	{
		down_right->state = BURNING;
	}
	if(down_left != nullptr && handler->material_props[down_left->material]->reaction & FLAMMABLE)
	{
		down_left->state = BURNING;
	}
	if(left != nullptr && handler->material_props[left->material]->reaction & FLAMMABLE)
	{
		left->state = BURNING;
	}
	if(right != nullptr && handler->material_props[right->material]->reaction & FLAMMABLE)
	{
		right->state = BURNING;
	}


}

void Elements::update_acid(ELEMENT_UPDATE_ARGS)
{

	vector2 mat_pos{material->position.x, material->position.y};
	Material* up = handler->in_world(mat_pos.x, mat_pos.y - 1)? handler->get_material(material->position.x, material->position.y - 1): nullptr;
	Material* down = handler->in_world(mat_pos.x, mat_pos.y + 1)? handler->get_material(material->position.x, material->position.y + 1): nullptr;
	Material* up_left = handler->in_world(mat_pos.x - 1, mat_pos.y - 1)? handler->get_material(material->position.x - 1, material->position.y - 1): nullptr;
	Material* up_right = handler->in_world(mat_pos.x + 1, mat_pos.y - 1)? handler->get_material(material->position.x + 1, material->position.y - 1): nullptr;
	Material* left = handler->in_world(mat_pos.x - 1, mat_pos.y)? handler->get_material(material->position.x - 1, material->position.y): nullptr;
	Material* right = handler->in_world(mat_pos.x + 1, mat_pos.y)? handler->get_material(material->position.x + 1, material->position.y): nullptr;
	Material* down_left = handler->in_world(mat_pos.x - 1, mat_pos.y + 1)? handler->get_material(material->position.x - 1, material->position.y + 1): nullptr;
	Material* down_right = handler->in_world(mat_pos.x + 1, mat_pos.y + 1)? handler->get_material(material->position.x + 1, material->position.y + 1): nullptr;

	if(up != nullptr && handler->material_props[up->material]->reaction & ACID_DESTROY)
	{
		up->health -= 10;
		if(up->health <= 0)
		{
			handler->destroy_material(up);
		}
	}
	if(down != nullptr && handler->material_props[down->material]->reaction & ACID_DESTROY)
	{
		down->health -= 10;
		if(down->health <= 0)
		{
			handler->destroy_material(down);
		}
	}
	if(up_left != nullptr && handler->material_props[up_left->material]->reaction & ACID_DESTROY)
	{
		up_left->health -= 10;
		if(up_left->health <= 0)
		{
			handler->destroy_material(up_left);
		}
	}
	if(up_right != nullptr && handler->material_props[up_right->material]->reaction & ACID_DESTROY)
	{
		up_right->health -= 10;
		if(up_right->health <= 0)
		{
			handler->destroy_material(up_right);
		}
	}
	if(down_right != nullptr && handler->material_props[down_right->material]->reaction & ACID_DESTROY)
	{
		down_right->health -= 10;
		if(down_right->health <= 0)
		{
			handler->destroy_material(down_right);
		}
	}
	if(down_left != nullptr && handler->material_props[down_left->material]->reaction & ACID_DESTROY)
	{
		down_left->health -= 10;
		if(down_left->health <= 0)
		{
			handler->destroy_material(down_left);
		}
	}
	if(left != nullptr && handler->material_props[left->material]->reaction & ACID_DESTROY)
	{
		left->health -= 10;
		if(left->health <= 0)
		{
			handler->destroy_material(left);
		}
	}
	if(right != nullptr && handler->material_props[right->material]->reaction & ACID_DESTROY)
	{
		right->health -= 10;
		if(right->health <= 0)
		{
			handler->destroy_material(right);
		}
	}

}

void Elements::update_oil(ELEMENT_UPDATE_ARGS){

	if(material->state != BURNING)
	{
		return;
	}

	if(rand() % 2 == 1)
	{
		material->health -= 5;
	}

	if(material->health <= 0)
	{
		handler->remove_from_anim_list(material);
		handler->destroy_material(material);
		return;
	}

	auto it = handler->animation_list.find(material);

	if(it == handler->animation_list.end() && material->tex_offset == handler->tex_coords.OIL && rand () % 10 < 5)
	{
		handler->animation_list[material] = {handler->tex_coords.FIRE, 33};
	}
	else if(it == handler->animation_list.end() && material->tex_offset == handler->tex_coords.FIRE && rand() % 10 < 5)
	{
		handler->animation_list[material] = {handler->tex_coords.FIRE_CHANGE_COLOR, 33};
	}
	else if(it == handler->animation_list.end() && material->tex_offset == handler->tex_coords.FIRE_CHANGE_COLOR && rand() % 10 < 3)
	{
		handler->animation_list[material] = {handler->tex_coords.OIL, 33};
	}

	vector2 mat_pos{material->position.x, material->position.y};
	Material* up = handler->in_world(mat_pos.x, mat_pos.y - 1)? handler->get_material(material->position.x, material->position.y - 1): nullptr;
	Material* down = handler->in_world(mat_pos.x, mat_pos.y + 1)? handler->get_material(material->position.x, material->position.y + 1): nullptr;
	Material* up_left = handler->in_world(mat_pos.x - 1, mat_pos.y - 1)? handler->get_material(material->position.x - 1, material->position.y - 1): nullptr;
	Material* up_right = handler->in_world(mat_pos.x + 1, mat_pos.y - 1)? handler->get_material(material->position.x + 1, material->position.y - 1): nullptr;
	Material* left = handler->in_world(mat_pos.x - 1, mat_pos.y)? handler->get_material(material->position.x - 1, material->position.y): nullptr;
	Material* right = handler->in_world(mat_pos.x + 1, mat_pos.y)? handler->get_material(material->position.x + 1, material->position.y): nullptr;
	Material* down_left = handler->in_world(mat_pos.x - 1, mat_pos.y + 1)? handler->get_material(material->position.x - 1, material->position.y + 1): nullptr;
	Material* down_right = handler->in_world(mat_pos.x + 1, mat_pos.y + 1)? handler->get_material(material->position.x + 1, material->position.y + 1): nullptr;

	ChunkHandler::Chunk* mat_chunk;
	if(up != nullptr && handler->material_props[up->material]->reaction & FLAMMABLE)
	{
		up->state = BURNING;
		mat_chunk = handler->get_chunk(up->position.x, up->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(down != nullptr && handler->material_props[down->material]->reaction & FLAMMABLE)
	{
		down->state = BURNING;
		mat_chunk = handler->get_chunk(down->position.x, down->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(up_left != nullptr && handler->material_props[up_left->material]->reaction & FLAMMABLE)
	{
		up_left->state = BURNING;
		mat_chunk = handler->get_chunk(up_left->position.x, up_left->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(up_right != nullptr && handler->material_props[up_right->material]->reaction & FLAMMABLE)
	{
		up_right->state = BURNING;
		mat_chunk = handler->get_chunk(up_right->position.x, up_right->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(down_right != nullptr && handler->material_props[down_right->material]->reaction & FLAMMABLE)
	{
		down_right->state = BURNING;
		mat_chunk = handler->get_chunk(down_right->position.x, down_right->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(down_left != nullptr && handler->material_props[down_left->material]->reaction & FLAMMABLE)
	{
		down_left->state = BURNING;
		mat_chunk = handler->get_chunk(down_left->position.x, down_left->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(left != nullptr && handler->material_props[left->material]->reaction & FLAMMABLE)
	{
		left->state = BURNING;
		mat_chunk = handler->get_chunk(left->position.x, left->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(right != nullptr && handler->material_props[right->material]->reaction & FLAMMABLE)
	{
		right->state = BURNING;
		mat_chunk = handler->get_chunk(right->position.x, right->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}

}


void Elements::update_flammable_gas(ELEMENT_UPDATE_ARGS)
{

	Material* displace_mat;
	if(handler->in_world(material->position.x, material->position.y - 1))
	{
		displace_mat = handler->get_material(material->position.x, material->position.y - 1);
	}

	if(displace_mat != nullptr && displace_mat->material != SMOKE && handler->material_props[displace_mat->material]->property != STATIC)
	{
		handler->swap_list.push_back({material->position, displace_mat->position});
		return;
	}

	if(material->state != BURNING)
	{
		return;
	}

	material->health -= 10;
	if(material->health <= 0)
	{
		handler->remove_from_anim_list(material);
		handler->destroy_material(material);
		return;
	}

	auto it = handler->animation_list.find(material);
	if(it == handler->animation_list.end() && material->tex_offset == handler->tex_coords.FL_GAS)
	{
		handler->animation_list[material] = {handler->tex_coords.FIRE, 1};
	}




	vector2 mat_pos{material->position.x, material->position.y};
	Material* up = handler->in_world(mat_pos.x, mat_pos.y - 1)? handler->get_material(material->position.x, material->position.y - 1): nullptr;
	Material* down = handler->in_world(mat_pos.x, mat_pos.y + 1)? handler->get_material(material->position.x, material->position.y + 1): nullptr;
	Material* up_left = handler->in_world(mat_pos.x - 1, mat_pos.y - 1)? handler->get_material(material->position.x - 1, material->position.y - 1): nullptr;
	Material* up_right = handler->in_world(mat_pos.x + 1, mat_pos.y - 1)? handler->get_material(material->position.x + 1, material->position.y - 1): nullptr;
	Material* left = handler->in_world(mat_pos.x - 1, mat_pos.y)? handler->get_material(material->position.x - 1, material->position.y): nullptr;
	Material* right = handler->in_world(mat_pos.x + 1, mat_pos.y)? handler->get_material(material->position.x + 1, material->position.y): nullptr;
	Material* down_left = handler->in_world(mat_pos.x - 1, mat_pos.y + 1)? handler->get_material(material->position.x - 1, material->position.y + 1): nullptr;
	Material* down_right = handler->in_world(mat_pos.x + 1, mat_pos.y + 1)? handler->get_material(material->position.x + 1, material->position.y + 1): nullptr;

	ChunkHandler::Chunk* mat_chunk;
	if(up != nullptr && handler->material_props[up->material]->reaction & FLAMMABLE)
	{
		up->state = BURNING;
		mat_chunk = handler->get_chunk(up->position.x, up->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(down != nullptr && handler->material_props[down->material]->reaction & FLAMMABLE)
	{
		down->state = BURNING;
		mat_chunk = handler->get_chunk(down->position.x, down->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(up_left != nullptr && handler->material_props[up_left->material]->reaction & FLAMMABLE)
	{
		up_left->state = BURNING;
		mat_chunk = handler->get_chunk(up_left->position.x, up_left->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(up_right != nullptr && handler->material_props[up_right->material]->reaction & FLAMMABLE)
	{
		up_right->state = BURNING;
		mat_chunk = handler->get_chunk(up_right->position.x, up_right->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(down_right != nullptr && handler->material_props[down_right->material]->reaction & FLAMMABLE)
	{
		down_right->state = BURNING;
		mat_chunk = handler->get_chunk(down_right->position.x, down_right->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(down_left != nullptr && handler->material_props[down_left->material]->reaction & FLAMMABLE)
	{
		down_left->state = BURNING;
		mat_chunk = handler->get_chunk(down_left->position.x, down_left->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(left != nullptr && handler->material_props[left->material]->reaction & FLAMMABLE)
	{
		left->state = BURNING;
		mat_chunk = handler->get_chunk(left->position.x, left->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(right != nullptr && handler->material_props[right->material]->reaction & FLAMMABLE)
	{
		right->state = BURNING;
		mat_chunk = handler->get_chunk(right->position.x, right->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
}

vector2 Elements::solid_physics_update(ELEMENT_UPDATE_ARGS, float inertia_chance, float friction_constant)
{
	vector2 grid_vel;
	round_velocity(&material->velocity, &grid_vel);

	Material* colliding_material = nullptr;
	vector2 updated_pos = pos_update(&material->position, &material->velocity, &colliding_material);
	vector2 colliding_mat_pos = colliding_material? colliding_material->position: (vector2){-1, -1};
	vector2 mat_pos = material->position;
	//updated_pos = updated_pos.x == -1? mat_pos: updated_pos;

	//Direction collision occurs
	bool collision_below;
	bool collision_diag_right;
	bool collision_diag_left;
	bool collision_left;
	bool collision_right;

	if(updated_pos.x != -1)
	{
		collision_below = colliding_material && colliding_mat_pos.x == updated_pos.x && colliding_mat_pos.y > updated_pos.y;
		collision_diag_right = colliding_material && colliding_mat_pos.x > updated_pos.x && colliding_mat_pos.y > updated_pos.y;
		collision_diag_left = colliding_material && colliding_mat_pos.x < updated_pos.x && colliding_mat_pos.y > updated_pos.y;
		collision_left = colliding_material && colliding_mat_pos.x < updated_pos.x && colliding_mat_pos.y == updated_pos.y;
		collision_right = colliding_material && colliding_mat_pos.x > updated_pos.x && colliding_mat_pos.y == updated_pos.y;
	}
	else
	{
		collision_below = colliding_material && colliding_mat_pos.x == mat_pos.x && colliding_mat_pos.y > mat_pos.y;
		collision_diag_right = colliding_material && colliding_mat_pos.x > mat_pos.x && colliding_mat_pos.y > mat_pos.y;
		collision_diag_left = colliding_material && colliding_mat_pos.x < mat_pos.x && colliding_mat_pos.y > mat_pos.y;
		collision_left = colliding_material && colliding_mat_pos.x < mat_pos.x && colliding_mat_pos.y == mat_pos.y;
		collision_right = colliding_material && colliding_mat_pos.x > mat_pos.x && colliding_mat_pos.y == mat_pos.y;
	}

	//Direction particle is falling
	bool falling_down = grid_vel.x == 0 && grid_vel.y > 0;
	bool falling_diag_left = grid_vel.x < 0 && grid_vel.y > 0;
	bool falling_diag_right = grid_vel.x > 0 && grid_vel.y > 0;

	assert((collision_below ^ collision_diag_right ^ collision_diag_left ^ collision_left ^ collision_right) ^ 
				(!collision_below && !collision_diag_right && !collision_diag_left && !collision_left && !collision_right));

	if(updated_pos.x == mat_pos.x && updated_pos.y == mat_pos.y && colliding_material)
	{
		std::cout << "Velocity is: " << print_vel(material->velocity.x, material->velocity.y) << std::endl;
	}

	if(collision_below)
	{
		Material* left_colliding_material = handler->in_world(colliding_mat_pos.x - 1, colliding_mat_pos.y)? handler->get_material(colliding_mat_pos.x - 1, colliding_mat_pos.y): dummy_material; 
		Material* right_colliding_material = handler->in_world(colliding_mat_pos.x + 1, colliding_mat_pos.y)? handler->get_material(colliding_mat_pos.x + 1, colliding_mat_pos.y): dummy_material;
		Material* left_material = handler->in_world(colliding_mat_pos.x - 1, colliding_mat_pos.y - 1)? handler->get_material(colliding_mat_pos.x - 1, colliding_mat_pos.y - 1): dummy_material;
		Material* right_material = handler->in_world(colliding_mat_pos.x + 1, colliding_mat_pos.y - 1)? handler->get_material(colliding_mat_pos.x + 1, colliding_mat_pos.y - 1): dummy_material;

		bool grounded = left_colliding_material && left_colliding_material != dummy_material && right_colliding_material && right_colliding_material != dummy_material;
		bool blocked = left_material && left_material != dummy_material && right_material && right_material != dummy_material;

		if(!left_colliding_material && !right_colliding_material && falling_down)
		{
			updated_pos.x = prob_gen(0.5f)? colliding_mat_pos.x + 1: colliding_mat_pos.x - 1;
			updated_pos.y = colliding_mat_pos.y;
		}
		else if(!left_colliding_material && falling_down)
		{
			updated_pos.x = colliding_mat_pos.x - 1;
			updated_pos.y = colliding_mat_pos.y;
		}
		else if(!right_colliding_material && falling_down)
		{
			updated_pos.x = colliding_mat_pos.x + 1;
			updated_pos.y = colliding_mat_pos.y;
		}

		if(!right_colliding_material && falling_diag_right)
		{
			updated_pos.x = colliding_mat_pos.x + 1;
			updated_pos.y = colliding_mat_pos.y;
		}
		else if(!left_colliding_material && falling_diag_left)
		{
			updated_pos.x = colliding_mat_pos.x - 1;
			updated_pos.y = colliding_mat_pos.y;
		}

		if(grounded && !left_material && !right_material && falling_down)
		{
			material->phys_state = SLIDING;
			material->velocity.x = prob_gen(0.5f)? material->velocity.y/10.0f: -material->velocity.y/10.0f;
			material->velocity.y = 1.0f;
		}
		else if(grounded && !left_material && falling_down)
		{
			material->phys_state = SLIDING;
			material->velocity.x = -material->velocity.y/10.0f;
			material->velocity.y = 1.0f;
		}
		else if(grounded && !right_material && falling_down)
		{
			material->phys_state = SLIDING;
			material->velocity.x = material->velocity.y/10.0f;
			material->velocity.y = 1.0f;
		}
		else if(grounded && !right_material && falling_diag_right)
		{
			material->phys_state = SLIDING;
			material->velocity.x += material->velocity.y/10.0f;
			material->velocity.y = 1.0f;
		}
		else if(grounded && !left_material && falling_diag_left)
		{
			material->phys_state = SLIDING;
			material->velocity.x -= material->velocity.y/10.0f;
			material->velocity.y = 1.0f;
		}
		else if(blocked)
		{
			material->phys_state = RESTING;
			material->velocity.x = 0;
			material->velocity.y = 1.0f;
		}
		

		round_velocity(&material->velocity, &grid_vel);
	}
	else if(collision_diag_left)
	{
		assert(falling_diag_left);

		Material* right_colliding_material = handler->in_world(colliding_mat_pos.x + 1, colliding_mat_pos.y)? handler->get_material(colliding_mat_pos.x + 1, colliding_mat_pos.y): dummy_material;
		Material* left_material = handler->in_world(colliding_mat_pos.x, colliding_mat_pos.y - 1)? handler->get_material(colliding_mat_pos.x, colliding_mat_pos.y - 1): dummy_material;
		bool blocked = right_colliding_material && right_colliding_material != dummy_material && left_material && left_material != dummy_material;

		if(!right_colliding_material)
		{
			updated_pos.x = colliding_mat_pos.x + 1;
			updated_pos.y = colliding_mat_pos.y;
			material->velocity.x = 0;
		}
		else if(!left_material)
		{
			updated_pos.x = colliding_mat_pos.x;
			updated_pos.y = colliding_mat_pos.y - 1;
		}
		else if(blocked)
		{
			material->velocity.y = 1.0f;
			material->velocity.x = 0;
			material->phys_state = RESTING;
		}
		round_velocity(&material->velocity, &grid_vel);
	}
	else if(collision_diag_right)
	{
		assert(falling_diag_right);

		Material* left_colliding_material = handler->in_world(colliding_mat_pos.x - 1, colliding_mat_pos.y)? handler->get_material(colliding_mat_pos.x - 1, colliding_mat_pos.y): dummy_material;
		Material* right_material = handler->in_world(colliding_mat_pos.x, colliding_mat_pos.y - 1)? handler->get_material(colliding_mat_pos.x, colliding_mat_pos.y - 1): dummy_material;
		bool blocked = left_colliding_material && left_colliding_material != dummy_material && right_material && right_material != dummy_material;

		if(!left_colliding_material)
		{
			updated_pos.x = colliding_mat_pos.x - 1;
			updated_pos.y = colliding_mat_pos.y;
			material->velocity.x = 0;
		}
		else if(!right_material)
		{
			updated_pos.x = colliding_mat_pos.x;
			updated_pos.y = colliding_mat_pos.y - 1;
		}
		else if(blocked)
		{
			material->velocity.y = 1.0f;
			material->velocity.x = 0;
			material->phys_state = RESTING;
		}
		round_velocity(&material->velocity, &grid_vel);
	}
	else if(collision_left || collision_right)
	{
		material->velocity.x = 0;
		round_velocity(&material->velocity, &grid_vel);
	}


	if(material->phys_state == FREE_FALLING)
	{
		Material* left_material = handler->in_world(updated_pos.x - 1, updated_pos.y)? handler->get_material(updated_pos.x - 1, updated_pos.y): dummy_material;
		Material* right_material = handler->in_world(updated_pos.x + 1, updated_pos.y)? handler->get_material(updated_pos.x + 1, updated_pos.y): dummy_material;

		if(left_material && left_material != dummy_material && left_material->phys_state == RESTING && prob_gen(inertia_chance))
		{
			left_material->phys_state = FREE_FALLING;
		}
		if(right_material && right_material != dummy_material && right_material->phys_state == RESTING && prob_gen(inertia_chance))
		{
			right_material->phys_state = FREE_FALLING;
		}
	}


	if(material->phys_state == FREE_FALLING && !collision_below)
	{
		material->velocity.y += g_force * dT;
	}

	if((material->phys_state == SLIDING || material->phys_state == FREE_FALLING) && grid_vel.x != 0)
	{
		float new_x = grid_vel.x > 0? material->velocity.x - friction_constant * g_force: material->velocity.x + friction_constant * g_force;
		if((float)grid_vel.x * new_x < 0)
		{
			material->velocity.x = 0;
			material->phys_state = RESTING;
		}
		else if(within_eps(new_x, 0, 0.5f) || within_eps(new_x, 0, -0.5f))
		{
			material->velocity.x = 0;
			material->phys_state = RESTING;
		}
		else{
			material->velocity.x = new_x;
		}
	}


	if(handler->in_world(mat_pos.x, mat_pos.y + 1) && !handler->get_material(mat_pos.x, mat_pos.y + 1) 
		&& (material->phys_state == RESTING || material->phys_state == SLIDING))
	{
		material->velocity.y = 1.0f;
		material->phys_state = FREE_FALLING;
		round_velocity(&material->velocity, &grid_vel);
	}

	if(updated_pos.x == world_width - 1 && grid_vel.x > 0)
	{
		material->velocity.x = 0;
		material->phys_state = RESTING;
		round_velocity(&material->velocity, &grid_vel);
	}
	else if(updated_pos.x == 0 && grid_vel.x < 0)
	{
		material->velocity.x = 0;
		material->phys_state = RESTING;
		round_velocity(&material->velocity, &grid_vel);
	}

	if(updated_pos.x == world_height - 1 && grid_vel.y > 0)
	{
		material->velocity.y = 1.0f;
		material->phys_state = RESTING;
		round_velocity(&material->velocity, &grid_vel);
	}


	if(grid_vel.x == 0 && grid_vel.y == 0)
	{
		material->velocity.x = 0;
		material->velocity.y = 1.0f;
		material->phys_state = RESTING;
	}


	clamp(material->velocity.x, 0.01f, 0);
	clamp(material->velocity.y, 0.01f, 0);


	return updated_pos;
}

vector2 Elements::pos_update(vector2* pos, fvector2* velocity, Material** colliding_material)
{
	if(velocity->x == 0 && velocity->y == 0)
	{
		return {-1, -1};
	}

	int vel_x = velocity->x > 0? static_cast<int>(velocity->x + 0.5f): static_cast<int>(velocity->x - 0.5f);
	int vel_y = velocity->y > 0? static_cast<int>(velocity->y + 0.5f): static_cast<int>(velocity->y - 0.5f);

	if(vel_x == 0 && vel_y == 0)
	{
		return {-1, -1};
	}

	vector2 update_pos {pos->x + vel_x, pos->y + vel_y};

	if(!handler->in_world(update_pos.x, update_pos.y))
	{
		update_pos.x = update_pos.x < 0? 0: update_pos.x;

		update_pos.x = update_pos.x >= world_width? world_width - 1: update_pos.x;
		update_pos.y = update_pos.y < 0? 0: update_pos.y;
		update_pos.y = update_pos.y >= world_height? world_height - 1: update_pos.y;
	}

	int x_offset = update_pos.x - pos->x;
	int y_offset = update_pos.y - pos->y;
	int offset = 1;
	float slope = x_offset != 0 && y_offset != 0? ((float)y_offset)/((float)x_offset): 0.0f;

	if(x_offset == 0 && y_offset == 0)
	{
		return {-1, -1};
	}

	if(slope == 0.0f && y_offset != 0)
	{
		while(offset <= std::abs(y_offset) && handler->in_world(pos->x, pos->y + offset) && !handler->get_material(pos->x, pos->y + offset)){
			offset ++;
		}

		if(colliding_material)
		{
			*colliding_material = y_offset > 0? handler->get_material(pos->x, pos->y + offset): handler->get_material(pos->x, pos->y - offset);
		}

		if(offset == 1)
		{
			return {-1, -1};
		}


		

		return {pos->x, y_offset > 0? pos->y + offset - 1: pos->y - offset + 1};
	}

	if(slope == 0.0f && x_offset != 0)
	{
		while(offset <= std::abs(x_offset) && handler->in_world(pos->x + offset, pos->y) && !handler->get_material(pos->x + offset, pos->y)){
			offset ++;
		}

		if(colliding_material)
		{
			*colliding_material = x_offset > 0? handler->get_material(pos->x + offset, pos->y): handler->get_material(pos->x - offset, pos->y);
		}

		if(offset == 1)
		{
			return {-1, -1};
		}


		return {x_offset > 0? pos->x + offset - 1: pos->x - offset + 1, pos->y};
	}

	assert(slope != 0.0f);

	bool right = update_pos.x > pos->x;

	float y_intercept = ((float)pos->y) - (slope * ((float)pos->x));


	int x_coord = pos->x + offset;
	int y_coord = (int)(slope * ((float)x_coord) + y_intercept);


	while(offset <= std::abs(x_offset) && handler->in_world(x_coord, y_coord) && !handler->get_material(x_coord, y_coord))
	{
		offset ++;
		x_coord = right? pos->x + offset: pos->x - offset;
		y_coord = (int)(slope * ((float)x_coord) + y_intercept);
	}

	if(colliding_material)
	{
		*colliding_material = handler->get_material(x_coord, y_coord);
	}

	if(offset == 1)
	{
		return {-1, -1};
	}


	return {slope > 0? pos->x + offset - 1: pos->x - offset + 1, right? (int)(slope * ((float)x_coord - 1) + y_intercept): (int)(slope * ((float)x_coord + 1) + y_intercept)};
}

void Elements::init(ChunkHandler* handler, int wW, int wH){
	this->handler = handler;
	world_width = wW;
	world_height = wH;
	gen.seed(rd());
}
