#include "Elements.hpp"
#include "ChunkHandler.hpp"
#include <cstdint>
#include <iostream>
#include <random>

void Elements::clamp_velocity(fvector2* velocity, float delta)
{
	velocity->x = velocity->x <= 0.5f? 0.0f: velocity->x;
	velocity->y = velocity->y <= 0.5f? 0.0f: velocity->y;
}

bool Elements::prob_gen(float chance)
{
	random_generator = std::bernoulli_distribution(chance);
	return random_generator(gen);
}

void Elements::update_sand(ELEMENT_UPDATE_ARGS)
{
	//Try to move
	vector2 updated_pos = solid_physics_update(material, dT, 0.85f, 0.3f);
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

	int vel_x = material->velocity.x >= 0? static_cast<int>(material->velocity.x + 0.5f): static_cast<int>(material->velocity.x - 0.5f);
	int vel_y = material->velocity.y >= 0? static_cast<int>(material->velocity.y + 0.5f): static_cast<int>(material->velocity.y - 0.5f);

	if(vel_y > 0 && prob_gen(inertia_chance))
	{
		Material* left_material;
		Material* right_material;

		if(handler->in_world(material->position.x - 1, material->position.y) && (left_material = handler->get_material(material->position.x - 1, material->position.y)) && std::abs(left_material->velocity.x) < 0.5f && left_material->velocity.y < 0.5f)
		{
			left_material->velocity.y = 1.0f;
		}
		if(handler->in_world(material->position.x + 1, material->position.y) && (right_material = handler->get_material(material->position.x + 1, material->position.y)) && std::abs(right_material->velocity.x) < 0.5f && right_material->velocity.y < 0.5f)
		{
			right_material->velocity.y = 1.0f;
		}
	}

	vector2 max_pos = {material->position.x + vel_x, material->position.y + vel_y}; 
	if(!handler->in_world(max_pos.x, max_pos.y))
	{
		max_pos.x = max_pos.x < 0? 0: max_pos.x;
		max_pos.x = max_pos.x >= world_width? world_width - 1: max_pos.x;
		max_pos.y = max_pos.y < 0? 0: max_pos.y;
		max_pos.y = max_pos.y >= world_height? world_height - 1: max_pos.y;
	}

	Material* colliding_material = nullptr;
	vector2 updated_pos = pos_update(&material->position, &material->velocity, &colliding_material);

	if(colliding_material && handler->material_props[colliding_material->material]->mat_state == SOLID && vel_y > 0 && vel_x == 0)
	{
		bool no_left_colliding_material = handler->in_world(colliding_material->position.x - 1, colliding_material->position.y) 
			&& !handler->get_material(colliding_material->position.x - 1, colliding_material->position.y);
		bool no_right_colliding_material = handler->in_world(colliding_material->position.x + 1, colliding_material->position.y) 
			&& !handler->get_material(colliding_material->position.x + 1, colliding_material->position.y);	

		bool no_left_material = handler->in_world(colliding_material->position.x - 1, colliding_material->position.y - 1) && !handler->get_material(colliding_material->position.x - 1, colliding_material->position.y - 1);
		bool no_right_material = handler->in_world(colliding_material->position.x + 1, colliding_material->position.y - 1) && !handler->get_material(colliding_material->position.x + 1, colliding_material->position.y - 1);


		if(no_left_colliding_material && no_right_colliding_material) 
		{
			updated_pos.x = prob_gen(0.5f)? colliding_material->position.x + 1: colliding_material->position.x - 1;
			updated_pos.y = colliding_material->position.y;
		}
		else if(no_left_colliding_material)
		{
			updated_pos.x = colliding_material->position.x - 1;
			updated_pos.y = colliding_material->position.y;
		}
		else if(no_right_colliding_material)
		{
			updated_pos.x = colliding_material->position.x + 1;
			updated_pos.y = colliding_material->position.y;
		}
		else if(no_left_material && no_right_material)
		{
			material->velocity.x = prob_gen(0.5f)? material->velocity.y/10.0f: -material->velocity.y/10.0f;
			material->velocity.y = 0.0f;
		}
		else if(no_right_material)
		{
			material->velocity.x = material->velocity.y/10.0f;
			material->velocity.y = 0.0f;
		}
		else if(no_left_material)
		{
			material->velocity.x = -material->velocity.y/10.0f;
			material->velocity.y = 0.0f;
		}
		else{
			material->velocity.x = 0.0f;
		}
		return updated_pos;
	}
	else if(colliding_material && handler->material_props[colliding_material->material]->mat_state == SOLID && vel_y > 0 && vel_x > 0
		&& colliding_material->position.x > updated_pos.x && colliding_material->position.y == updated_pos.y)
	{
		bool no_diag_left_colliding_material = handler->in_world(colliding_material->position.x - 1, colliding_material->position.y + 1) 
			&& !handler->get_material(colliding_material->position.x - 1, colliding_material->position.y + 1);

		updated_pos.x = colliding_material->position.x -  1;
		updated_pos.y = colliding_material->position.y;

		if(no_diag_left_colliding_material)
		{
			material->velocity.x = 0;
		}
		else{
			material->velocity.x = 0;
			material->velocity.y = 0;
		}
		return updated_pos;
	}
	else if(colliding_material && handler->material_props[colliding_material->material]->mat_state == SOLID && vel_y > 0 && vel_x > 0
		&& colliding_material->position.x > updated_pos.x && colliding_material->position.y > updated_pos.y)
	{
		bool no_material_above_colliding_material = handler->in_world(colliding_material->position.x, colliding_material->position.y - 1) 
			&& !handler->get_material(colliding_material->position.x, colliding_material->position.y - 1);
		bool no_material_left_colliding_material = handler->in_world(colliding_material->position.x - 1, colliding_material->position.y)
			&& !handler->get_material(colliding_material->position.x - 1, colliding_material->position.y);

		if(no_material_left_colliding_material)
		{
			updated_pos.x = colliding_material->position.x - 1;
			updated_pos.y = colliding_material->position.y;
			material->velocity.x = 0;
		}
		else if(no_material_above_colliding_material && !no_material_left_colliding_material)
		{
			updated_pos.x = colliding_material->position.x - 1;
			updated_pos.y = colliding_material->position.y - 1;
			material->velocity.x += material->velocity.y/10.0f;
			material->velocity.y = 0;
		}
		else if(!no_material_above_colliding_material && !no_material_left_colliding_material)
		{
			updated_pos.x = colliding_material->position.x - 1;
			updated_pos.y = colliding_material->position.y - 1;
			material->velocity.x = 0;
			material->velocity.y = 0;
		}
		return updated_pos;
	}
	else if(colliding_material && handler->material_props[colliding_material->material]->mat_state == SOLID && vel_y > 0 && vel_x > 0
		&& colliding_material->position.x == updated_pos.x && colliding_material->position.y > updated_pos.y)
	{
		bool no_right_colliding_material = handler->in_world(colliding_material->position.x + 1, colliding_material->position.y + 1)
			&& !handler->get_material(colliding_material->position.x + 1, colliding_material->position.y + 1);
		bool no_diag_up_right_colliding_material = handler->in_world(colliding_material->position.x + 1, colliding_material->position.y - 1)
			&& !handler->get_material(colliding_material->position.x + 1, colliding_material->position.y - 1);

		if(no_right_colliding_material && no_diag_up_right_colliding_material)
		{
			updated_pos.x = colliding_material->position.x + 1;
			updated_pos.y = colliding_material->position.y;
			material->velocity.x -= material->velocity.x/10.0f;
		}
		else if(!no_diag_up_right_colliding_material)
		{
			updated_pos.x = colliding_material->position.x;
			updated_pos.y = colliding_material->position.y - 1;
			material->velocity.x = 0;
			material->velocity.y = 0;
		}
		else if(!no_right_colliding_material && no_diag_up_right_colliding_material)
		{
			updated_pos.x = colliding_material->position.x;
			updated_pos.y = colliding_material->position.y - 1;
			material->velocity.x += material->velocity.y/10.0f;
			material->velocity.y = 0;
		}
		return updated_pos;
	}
	else if(colliding_material && handler->material_props[colliding_material->material]->mat_state == SOLID && vel_y > 0 && vel_x < 0
		&& colliding_material->position.x < updated_pos.x && colliding_material->position.y == updated_pos.y)
	{
		bool no_diag_right_colliding_material = handler->in_world(colliding_material->position.x + 1, colliding_material->position.y + 1) 
			&& !handler->get_material(colliding_material->position.x + 1, colliding_material->position.y + 1);

		updated_pos.x = colliding_material->position.x - 1;
		updated_pos.y = colliding_material->position.y;

		if(no_diag_right_colliding_material)
		{
			material->velocity.x = 0;
		}
		else{
			material->velocity.x = 0;
			material->velocity.y = 0;
		}
		return updated_pos;
	}
	else if(colliding_material && handler->material_props[colliding_material->material]->mat_state == SOLID && vel_y > 0 && vel_x < 0
		&& colliding_material->position.x < updated_pos.x && colliding_material->position.y > updated_pos.y)
	{
		bool no_material_above_colliding_material = handler->in_world(colliding_material->position.x, colliding_material->position.y - 1) 
			&& !handler->get_material(colliding_material->position.x, colliding_material->position.y - 1);
		bool no_material_right_colliding_material = handler->in_world(colliding_material->position.x + 1, colliding_material->position.y)
			&& !handler->get_material(colliding_material->position.x + 1, colliding_material->position.y);

		if(no_material_right_colliding_material)
		{
			updated_pos.x = colliding_material->position.x + 1;
			updated_pos.y = colliding_material->position.y;
			material->velocity.x = 0;
		}
		else if(no_material_above_colliding_material && !no_material_right_colliding_material)
		{
			updated_pos.x = colliding_material->position.x + 1;
			updated_pos.y = colliding_material->position.y + 1;
			material->velocity.x -= material->velocity.y/10.0f;
			material->velocity.y = 0;
		}
		else if(!no_material_above_colliding_material && !no_material_right_colliding_material)
		{
			updated_pos.x = colliding_material->position.x + 1;
			updated_pos.y = colliding_material->position.y + 1;
			material->velocity.x = 0;
			material->velocity.y = 0;
		}
		return updated_pos;
	}
	else if(colliding_material && handler->material_props[colliding_material->material]->mat_state == SOLID && vel_y > 0 && vel_x < 0
		&& colliding_material->position.x == updated_pos.x && colliding_material->position.y > updated_pos.y)
	{
		bool no_left_colliding_material = handler->in_world(colliding_material->position.x - 1, colliding_material->position.y + 1)
			&& !handler->get_material(colliding_material->position.x - 1, colliding_material->position.y + 1);
		bool no_diag_up_left_colliding_material = handler->in_world(colliding_material->position.x - 1, colliding_material->position.y - 1)
			&& !handler->get_material(colliding_material->position.x - 1, colliding_material->position.y - 1);

		if(no_left_colliding_material && no_diag_up_left_colliding_material)
		{
			updated_pos.x = colliding_material->position.x - 1;
			updated_pos.y = colliding_material->position.y;
			material->velocity.x -= material->velocity.x/10.0f;
		}
		else if(!no_diag_up_left_colliding_material)
		{
			updated_pos.x = colliding_material->position.x;
			updated_pos.y = colliding_material->position.y - 1;
			material->velocity.x = 0;
			material->velocity.y = 0;
		}
		else if(!no_left_colliding_material && no_diag_up_left_colliding_material)
		{
			updated_pos.x = colliding_material->position.x;
			updated_pos.y = colliding_material->position.y - 1;
			material->velocity.x -= material->velocity.y/10.0f;
			material->velocity.y = 0;
		}
		return updated_pos;
	}

	bool on_world_ground = updated_pos.y == world_height - 1;
	bool ground_exists = handler->in_world(updated_pos.x, updated_pos.y + 1) && handler->get_material(updated_pos.x, updated_pos.y);
	bool grounded = on_world_ground || ground_exists;


	//Gravity
	if(!colliding_material && material->position.y <= world_height - 1 && !grounded)
	{
		material->velocity.y += g_force * dT;
	}
	else
	{
		material->velocity.y = 0;
	}

	//Air resitance and friction, essentially no collision just decrease x no matter what y is
	if(!colliding_material && vel_x != 0)
	{
		material->velocity.x = material->velocity.x > 0? material->velocity.x - friction_constant * dT: material->velocity.x + friction_constant * dT;
	}



	//Collision with edge of the screen
	if(vel_y > 0 && updated_pos.y == world_height - 1)
	{
		material->velocity.x = prob_gen(0.5f)? material->velocity.y/2.0f: -material->velocity.y/2.0f;
		material->velocity.y = 0;
	}
	if(vel_x > 0 && updated_pos.x == world_width - 1)
	{
		material->velocity.x = 0;
	}
	else if(vel_x < 0 && updated_pos.x == 0)
	{
		material->velocity.x = 0;
	}

	clamp_velocity(&material->velocity, 0.01f);
	//Logic if material can move if it's y-velocity is zeroed 	return updated_pos;
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
		if(offset == 1)
		{
			return {-1, -1};
		}
		
		if(colliding_material)
		{
			*colliding_material = handler->get_material(pos->x, pos->y + offset);
		}

		return {pos->x, y_offset > 0? pos->y + offset - 1: pos->y - offset + 1};
	}

	if(slope == 0.0f && x_offset != 0)
	{
		while(offset <= std::abs(x_offset) && handler->in_world(pos->x + offset, pos->y) && !handler->get_material(pos->x + offset, pos->y)){
			offset ++;
		}
		if(offset == 1)
		{
			return {-1, -1};
		}

		if(colliding_material)
		{
			*colliding_material = handler->get_material(pos->x, pos->y + offset);
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

	if(offset == 1)
	{
		return {-1, -1};
	}

	if(colliding_material)
	{
		*colliding_material = handler->get_material(x_coord, y_coord);
	}


	return {slope > 0? pos->x + offset - 1: pos->x - offset + 1, right? (int)(slope * ((float)x_coord - 1) + y_intercept): (int)(slope * ((float)x_coord + 1) + y_intercept)};
}

void Elements::init(ChunkHandler* handler, int wW, int wH){
	this->handler = handler;
	world_width = wW;
	world_height = wH;
	gen.seed(rd());
}
