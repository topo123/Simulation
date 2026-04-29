#include "Elements.hpp"
#include "ChunkHandler.hpp"
#include <cstdint>
#include <iostream>
#include <random>
#include <string>

int down_collision_rest = 0;
int from_sliding_rest = 0;
int world_collision_rest = 0;
int natural_rest = 0;

std::string Elements::print_vel(float x, float y)
{
	return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
}


void Elements::update_sand(const float dT, const PhysicsMaterialMoveData* phys_move_data)
{
	Material* material = phys_move_data->org_material;
	vector2 updated_pos = solid_physics_update(phys_move_data, dT, 0.3f, 0.2f);
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

void Elements::update_smoke(const float dT, const PhysicsMaterialMoveData* phys_move_data)
{

	Material* material = phys_move_data->org_material;
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


void Elements::update_water(const float dT, const PhysicsMaterialMoveData* phys_move_data)
{
	Material* material = phys_move_data->org_material;
	vector2 updated_pos = fluid_physics_update(phys_move_data, dT, 6, 0.45);

	if(updated_pos.x != -1)
	{
		handler->move_list.push_back({material->position, updated_pos});
		return;
	}
}

void Elements::update_wood(const float dT, const PhysicsMaterialMoveData* phys_move_data)
{
	Material* material = phys_move_data->org_material;
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

	Chunk* mat_chunk;
	if(up != nullptr && handler->material_props[up->material]->reaction & FLAMMABLE)
	{
		up->state = BURNING;
		mat_chunk = handler->get_chunk(up->position.x, up->position.y);
	}
	if(down != nullptr && handler->material_props[down->material]->reaction & FLAMMABLE)
	{
		down->state = BURNING;
		mat_chunk = handler->get_chunk(down->position.x, down->position.y);
	}
	if(up_left != nullptr && handler->material_props[up_left->material]->reaction & FLAMMABLE)
	{
		up_left->state = BURNING;
		mat_chunk = handler->get_chunk(up_left->position.x, up_left->position.y);
	}
	if(up_right != nullptr && handler->material_props[up_right->material]->reaction & FLAMMABLE)
	{
		up_right->state = BURNING;
		mat_chunk = handler->get_chunk(up_right->position.x, up_right->position.y);
	}
	if(down_right != nullptr && handler->material_props[down_right->material]->reaction & FLAMMABLE)
	{
		down_right->state = BURNING;
		mat_chunk = handler->get_chunk(down_right->position.x, down_right->position.y);
	}
	if(down_left != nullptr && handler->material_props[down_left->material]->reaction & FLAMMABLE)
	{
		down_left->state = BURNING;
		mat_chunk = handler->get_chunk(down_left->position.x, down_left->position.y);
	}
	if(left != nullptr && handler->material_props[left->material]->reaction & FLAMMABLE)
	{
		left->state = BURNING;
		mat_chunk = handler->get_chunk(left->position.x, left->position.y);
	}
	if(right != nullptr && handler->material_props[right->material]->reaction & FLAMMABLE)
	{
		right->state = BURNING;
		mat_chunk = handler->get_chunk(right->position.x, right->position.y);
	}
}

void Elements::update_fire(const float dT, const PhysicsMaterialMoveData* phys_move_data)
{
	Material* material = phys_move_data->org_material;
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

void Elements::update_acid(const float dT, const PhysicsMaterialMoveData* phys_move_data)
{
	Material* material = phys_move_data->org_material;
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

void Elements::update_oil(const float dT, const PhysicsMaterialMoveData* phys_move_data){

	Material* material = phys_move_data->org_material;
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

	Chunk* mat_chunk;
	if(up != nullptr && handler->material_props[up->material]->reaction & FLAMMABLE)
	{
		up->state = BURNING;
		mat_chunk = handler->get_chunk(up->position.x, up->position.y);
	}
	if(down != nullptr && handler->material_props[down->material]->reaction & FLAMMABLE)
	{
		down->state = BURNING;
		mat_chunk = handler->get_chunk(down->position.x, down->position.y);
	}
	if(up_left != nullptr && handler->material_props[up_left->material]->reaction & FLAMMABLE)
	{
		up_left->state = BURNING;
		mat_chunk = handler->get_chunk(up_left->position.x, up_left->position.y);
	}
	if(up_right != nullptr && handler->material_props[up_right->material]->reaction & FLAMMABLE)
	{
		up_right->state = BURNING;
		mat_chunk = handler->get_chunk(up_right->position.x, up_right->position.y);
	}
	if(down_right != nullptr && handler->material_props[down_right->material]->reaction & FLAMMABLE)
	{
		down_right->state = BURNING;
		mat_chunk = handler->get_chunk(down_right->position.x, down_right->position.y);
	}
	if(down_left != nullptr && handler->material_props[down_left->material]->reaction & FLAMMABLE)
	{
		down_left->state = BURNING;
		mat_chunk = handler->get_chunk(down_left->position.x, down_left->position.y);
	}
	if(left != nullptr && handler->material_props[left->material]->reaction & FLAMMABLE)
	{
		left->state = BURNING;
		mat_chunk = handler->get_chunk(left->position.x, left->position.y);
	}
	if(right != nullptr && handler->material_props[right->material]->reaction & FLAMMABLE)
	{
		right->state = BURNING;
		mat_chunk = handler->get_chunk(right->position.x, right->position.y);
	}

}


void Elements::update_flammable_gas(const float dT, const PhysicsMaterialMoveData* phys_move_data)
{

	Material* material = phys_move_data->org_material;
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

	Chunk* mat_chunk;
	if(up != nullptr && handler->material_props[up->material]->reaction & FLAMMABLE)
	{
		up->state = BURNING;
		mat_chunk = handler->get_chunk(up->position.x, up->position.y);
	}
	if(down != nullptr && handler->material_props[down->material]->reaction & FLAMMABLE)
	{
		down->state = BURNING;
		mat_chunk = handler->get_chunk(down->position.x, down->position.y);
	}
	if(up_left != nullptr && handler->material_props[up_left->material]->reaction & FLAMMABLE)
	{
		up_left->state = BURNING;
		mat_chunk = handler->get_chunk(up_left->position.x, up_left->position.y);
	}
	if(up_right != nullptr && handler->material_props[up_right->material]->reaction & FLAMMABLE)
	{
		up_right->state = BURNING;
		mat_chunk = handler->get_chunk(up_right->position.x, up_right->position.y);
	}
	if(down_right != nullptr && handler->material_props[down_right->material]->reaction & FLAMMABLE)
	{
		down_right->state = BURNING;
		mat_chunk = handler->get_chunk(down_right->position.x, down_right->position.y);
	}
	if(down_left != nullptr && handler->material_props[down_left->material]->reaction & FLAMMABLE)
	{
		down_left->state = BURNING;
		mat_chunk = handler->get_chunk(down_left->position.x, down_left->position.y);
	}
	if(left != nullptr && handler->material_props[left->material]->reaction & FLAMMABLE)
	{
		left->state = BURNING;
		mat_chunk = handler->get_chunk(left->position.x, left->position.y);
	}
	if(right != nullptr && handler->material_props[right->material]->reaction & FLAMMABLE)
	{
		right->state = BURNING;
		mat_chunk = handler->get_chunk(right->position.x, right->position.y);
	}
}


vector2 Elements::fluid_physics_update(const PhysicsMaterialMoveData* phys_move_data, const float dT, const int fluid_spread, const float friction_constant)
{
}

vector2 Elements::solid_physics_update(const PhysicsMaterialMoveData* phys_move_data, const float dT, const float inertia_chance, const float friction_constant)
{
	vector2 final_pos = phys_move_data->new_pos;
	Material* material = phys_move_data->org_material;
	Material* hit_material = phys_move_data->hit_material;

	vector2 rounded_velocity;
	round_velocity(&material->velocity, &rounded_velocity);

	//Waking up neighors for updates


	
	//Code to determine what happens upon collision with the world's edges. 
	if(phys_move_data->hit_material != nullptr && (final_pos.x == world_width - 1 && final_pos.y == world_height - 1) && rounded_velocity.x > 0)
	{
		material->velocity.x = 0;
		material->velocity.y = 0;
		return final_pos;
	}
	else if(phys_move_data->hit_material != nullptr && (final_pos.x == 0 && final_pos.y == world_height - 1) && rounded_velocity.x < 0)
	{
		material->velocity.x = 0;
		material->velocity.y = 0;
		return final_pos;
	}
	else if(phys_move_data->hit_material != nullptr && final_pos.y == world_height - 1 && rounded_velocity.y > 0 && rounded_velocity.x != 0)
	{
		material->velocity.y = 0;
		material->velocity.x = rounded_velocity.x > 0? material->velocity.x + material->velocity.y/3.0f: material->velocity.x - material->velocity.y/3.0f;
		return final_pos;
	}
	else if(phys_move_data->hit_material != nullptr && final_pos.y == world_height - 1 && rounded_velocity.y > 0 && rounded_velocity.x == 0)
	{
		material->velocity.y = 0;
		material->velocity.x = rand() % 2 == 0? material->velocity.y/3.0f: -material->velocity.y/3.0f;
		return final_pos;
	}
	else if(phys_move_data->hit_material != nullptr && final_pos.x == 0  && rounded_velocity.x < 0)
	{
		material->velocity.x = 0;
		return final_pos;
	}
	if(phys_move_data->hit_material != nullptr && final_pos.x == world_width - 1 && rounded_velocity.x > 0)
	{
		material->velocity.x = 0;
		return final_pos;
	}

	//Code to determine what happens on collisions that are not at the world's edges.
	bool collision_below = hit_material->position.x == material->position.x && hit_material->position.y > material->position.y;
	bool collision_diag_right = hit_material->position.x > material->position.x && hit_material->position.y > material->position.y;
	bool collision_diag_left = hit_material->position.x < material->position.x && hit_material->position.y > material->position.y;
	bool collision_left = hit_material->position.x < material->position.x && hit_material->position.y == material->position.y;
	bool collision_right = hit_material->position.x > material->position.x && hit_material->position.y == material->position.y;

	if(collision_below)
	{
		bool left = handler->get_material(material->position.x - 1, material->position.y) && handler->in_world(material->position.x - 1, material->position.y);
		bool right = handler->get_material(material->position.x + 1, material->position.y) && handler->in_world(material->position.x - 1, material->position.y);
		bool down = handler->get_material(material->position.x, material->position.y) && handler->in_world(material->position.x - 1, material->position.y);
		bool diag_left = handler->get_material(material->position.x - 1, material->position.y + 1) && handler->in_world(material->position.x - 1, material->position.y);
		bool diag_right = handler->get_material(material->position.x + 1, material->position.y + 1) && handler->in_world(material->position.x - 1, material->position.y);
		bool falling_down = rounded_velocity.x == 0 && rounded_velocity.y > 0;

		if(left && down && !right && !diag_right && falling_down && prob_gen(inertia_chance))
		{
			final_pos.x = material->position.x + 1;
			final_pos.y = material->position.y + 1;
		}
		else if(right && down && !left && !diag_left && falling_down && prob_gen(inertia_chance))
		{
			final_pos.x = material->position.x - 1;
			final_pos.y = material->position.y + 1;
		}

		if(rounded_velocity.x == 0 && rounded_velocity.y > 0)
		{
			material->velocity.x = rand() % 2 == 0? -rounded_velocity.y/2.0f: rounded_velocity.y/2.0f;
		}
		else if(rounded_velocity.x != 0 && rounded_velocity.y > 0)
		{
			bool negative = material->velocity.x < 0;
			material->velocity.x += -(material->velocity.x/5.0f);  
			negative? clamp(material->velocity.x, -10.0f, 0): clamp(material->velocity.x, 0, 10.0f);
		}
		return final_pos;
	}
	else if(collision_diag_right)
	{
		material->velocity.x = 0;
		return final_pos;
	}
	else if(collision_diag_left)
	{
		material->velocity.x = 0;
		return final_pos;
	}
	else if(collision_left)
	{
		material->velocity.x = 0;
		return final_pos;
	}
	else if(collision_right && rounded_velocity.x > 0)
	{
		material->velocity.x = 0;
		return final_pos;
	}

	//Velocity sliding updates
	if(rounded_velocity.x != 0 && rounded_velocity.y == 0)
	{
		bool negative = material->velocity.x < 0;
		material->velocity.x += -(material->velocity.x/10.0f);
		negative? clamp(material->velocity.x, -10.0f, 0): clamp(material->velocity.x, 0, 10.0f);
	}

	//Modeling air resitance
	if(rounded_velocity.x != 0 && rounded_velocity.y > 0)
	{
		bool negative = material->velocity.x < 0;
		material->velocity.x += -(material->velocity.x/10.0f);
		negative? clamp(material->velocity.x, -10.0f, 0): clamp(material->velocity.x, 0, 10.0f);
	}

	//Gravity
	if(rounded_velocity.y > 0)
	{
		material->velocity.y += g_force * dT;
		clamp(material->velocity.y, 0, 10.0f);
	}

	//States where a material is in a position to behave a certain way but no velocity to support it. Like sand sliding off a platform but does not have a downward velocity.
	if(handler->get_material(final_pos.x, final_pos.y + 1) == nullptr)
	{
		material->velocity.y = 1;
	}

	//Special update related to inertia chance. 


	return final_pos;
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
			if(*colliding_material) assert(handler->in_world((*colliding_material)->position.x, (*colliding_material)->position.y));
		}

		if(offset == 1)
		{
			return {-1, -1};
		}

		return {pos->x, y_offset > 0? pos->y + offset - 1: pos->y - offset + 1};
	}

	if(slope == 0.0f && x_offset > 0)
	{
		while(offset <= std::abs(x_offset) && handler->in_world(pos->x + offset, pos->y) && !handler->get_material(pos->x + offset, pos->y)){
			offset ++;
		}

		if(colliding_material)
		{
			*colliding_material = handler->get_material(pos->x + offset, pos->y);
		}

		if(offset == 1) return {-1, -1};
		return {pos->x + offset - 1, pos->y};
	}
	else if(slope == 0 && x_offset < 0)
	{
		while(offset <= std::abs(x_offset) && handler->in_world(pos->x - offset, pos->y) && !handler->get_material(pos->x - offset, pos->y)){
			offset ++;
		}

		if(colliding_material)
		{
			*colliding_material = handler->get_material(pos->x - offset, pos->y);
		}


		if(offset == 1) return {-1, -1};
		return {pos->x - offset + 1, pos->y};
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
}
