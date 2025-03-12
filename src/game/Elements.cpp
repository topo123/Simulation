#include "Elements.hpp"
#include "ChunkHandler.hpp"
#include <cstdint>




bool Elements::update_down(ELEMENT_UPDATE_ARGS)
{
	if(!handler->in_world(material->position.x, material->position.y + 1) || handler->get_material(material->position.x, material->position.y + 1) != nullptr)
	{
		return false;
	}


	material->velocity.y = material->velocity.y + (g_force * dT);
	vector2 new_pos{material->position.x, material->position.y};
	int curr_offset = 1;
	const int num_cells_move = (int)(material->velocity.y + 0.5);

	while(curr_offset <= num_cells_move && handler->in_world(new_pos.x, new_pos.y + curr_offset) && handler->get_material(new_pos.x, new_pos.y + curr_offset) == nullptr){
		curr_offset ++;
	}

	if(curr_offset == 1)
	{
		material->velocity.y = 0;
		return false;
	}

	new_pos.y += curr_offset - 1;
	if(new_pos.y >= handler->world_height){
		new_pos.y = 599;
	}

	ChunkHandler::Move new_move {{material->position.x, material->position.y}, {new_pos.x, new_pos.y}};
	handler->move_list.push_back(new_move);
	return true;
}

bool Elements::update_up(Material* material)
{
	if(!handler->in_world(material->position.x, material->position.y - 1))
	{
		return false;
	}
	Material* check_material = handler->get_material(material->position.x, material->position.y - 1);


	if(check_material == nullptr)
	{
		vector2 new_pos {material->position.x, material->position.y - 1};
		ChunkHandler::Move new_move {{material->position.x, material->position.y}, {new_pos.x, new_pos.y}};
		handler->move_list.push_back(new_move);
		return true;
	}
	return false;
}

bool Elements::update_side_up(Material* material)
{

	uint8_t direction = rand() % 2;
	uint8_t can_move = false;

	if(direction == 0)
	{
		uint8_t left_clear = handler->in_world(material->position.x - 1, material->position.y) && handler->get_material(material->position.x - 1, material->position.y) == nullptr;
		uint8_t diag_left_clear = handler->in_world(material->position.x - 1, material->position.y - 1) && handler->get_material(material->position.x - 1, material->position.y - 1) == nullptr;
		can_move = left_clear && diag_left_clear;
	}
	else
{
		uint8_t right_clear = handler->in_world(material->position.x + 1, material->position.y) && handler->get_material(material->position.x + 1, material->position.y) == nullptr;
		uint8_t diag_right_clear = handler->in_world(material->position.x + 1, material->position.y - 1) && handler->get_material(material->position.x + 1, material->position.y - 1) == nullptr;
		can_move = right_clear && diag_right_clear;
	}

	if(!can_move && direction == 0)
	{
		uint8_t right_clear = handler->in_world(material->position.x + 1, material->position.y) && handler->get_material(material->position.x + 1, material->position.y) == nullptr;
		uint8_t diag_right_clear = handler->in_world(material->position.x + 1, material->position.y - 1) && handler->get_material(material->position.x + 1, material->position.y - 1) == nullptr;
		can_move = right_clear && diag_right_clear;
		direction = 1;
	}
	else if(!can_move && direction == 1)
	{
		uint8_t left_clear = handler->in_world(material->position.x - 1, material->position.y) && handler->get_material(material->position.x - 1, material->position.y) == nullptr;
		uint8_t diag_left_clear = handler->in_world(material->position.x - 1, material->position.y - 1) && handler->get_material(material->position.x - 1, material->position.y - 1) == nullptr;
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
		ChunkHandler::Move new_move {{material->position.x, material->position.y}, {new_pos.x, new_pos.y}};
		handler->move_list.push_back(new_move);
		return true;
	}
	else if(direction == 1)
	{
		vector2 new_pos {material->position.x + 1, material->position.y - 1};
		ChunkHandler::Move new_move {{material->position.x, material->position.y}, {new_pos.x, new_pos.y}};
		handler->move_list.push_back(new_move);
		return true;
	}
	return false;
}

bool Elements::update_side(Material* material)
{
	uint8_t left = material->velocity.x < 0;
	uint8_t right = material->velocity.x > 0; 
	if(!left && !right)
	{
		uint8_t direction = rand() % 2;
		left = direction == 0;
		right = direction == 1;
		material->velocity.x = left ? -fast_liquid_spread: fast_liquid_spread;
	}



	vector2 new_pos{material->position.x, material->position.y};
	const int num_cells_move = fast_liquid_spread;
	int curr_offset = 1;

	if(left) 
	{
		while(handler->in_world(new_pos.x - curr_offset, new_pos.y) && handler->get_material(new_pos.x - curr_offset, new_pos.y) == nullptr && curr_offset <= num_cells_move){
			curr_offset ++;
		}
	}
	if(right)
	{
		while(handler->in_world(new_pos.x + curr_offset, new_pos.y) && handler->get_material(new_pos.x + curr_offset, new_pos.y) == nullptr && curr_offset <= num_cells_move){
			curr_offset ++;
		}
	}

	if(curr_offset == 1){
		left = !left;
		right = !right;
		material->velocity.x = -material->velocity.x;
	}
	else if(left)
	{
		ChunkHandler::Move move {{material->position.x, material->position.y}, {new_pos.x - curr_offset + 1, new_pos.y}};
		handler->move_list.push_back(move);
		return true;
	}
	else if(right)
	{
		ChunkHandler::Move move {{material->position.x, material->position.y}, {new_pos.x + curr_offset - 1, new_pos.y}};
		handler->move_list.push_back(move);
		return true;
	}

	curr_offset = 1;
	if(left) 
	{
		while(handler->in_world(new_pos.x - curr_offset, new_pos.y) && handler->get_material(new_pos.x - curr_offset, new_pos.y) == nullptr && curr_offset <= num_cells_move){
			curr_offset ++;
		}
	}
	if(right)
	{
		while(handler->in_world(new_pos.x + curr_offset, new_pos.y) && handler->get_material(new_pos.x + curr_offset, new_pos.y) == nullptr && curr_offset <= num_cells_move){
			curr_offset ++;
		}
	}

	if(curr_offset == 1){
		material->velocity.x = 0;
		return false;
	}
	else if(left)
	{
		ChunkHandler::Move move {{material->position.x, material->position.y}, {new_pos.x - curr_offset + 1, new_pos.y}};
		handler->move_list.push_back(move);
		return true;
	}
	else if(right)
	{
		ChunkHandler::Move move {{material->position.x, material->position.y}, {new_pos.x + curr_offset - 1, new_pos.y}};
		handler->move_list.push_back(move);
		return true;
	}
	assert(false);
	return false;
}


bool Elements::update_side_down(Material* material)
{
	uint8_t direction = rand() % 2;
	uint8_t can_move = false;

	if(direction == 0)
	{
		uint8_t left_clear = handler->in_world(material->position.x -1, material->position.y) && handler->get_material(material->position.x - 1, material->position.y) == nullptr;
		uint8_t diag_left_clear = handler->in_world(material->position.x - 1, material->position.y + 1) && handler->get_material(material->position.x - 1, material->position.y + 1) == nullptr;
		can_move = left_clear && diag_left_clear;
	}
	else
{
		uint8_t right_clear = handler->in_world(material->position.x + 1, material->position.y) && handler->get_material(material->position.x + 1, material->position.y) == nullptr;
		uint8_t diag_right_clear = handler->in_world(material->position.x + 1, material->position.y + 1) && handler->get_material(material->position.x + 1, material->position.y + 1) == nullptr;
		can_move = right_clear && diag_right_clear;
	}

	if(!can_move && direction == 0)
	{
		uint8_t right_clear = handler->in_world(material->position.x + 1, material->position.y) && handler->get_material(material->position.x + 1, material->position.y) == nullptr;
		uint8_t diag_right_clear = handler->in_world(material->position.x + 1, material->position.y + 1) && handler->get_material(material->position.x + 1, material->position.y + 1) == nullptr;
		can_move = right_clear && diag_right_clear;
		direction = 1;
	}
	else if(!can_move && direction == 1)
	{
		uint8_t left_clear = handler->in_world(material->position.x -1, material->position.y) && handler->get_material(material->position.x - 1, material->position.y) == nullptr;
		uint8_t diag_left_clear = handler->in_world(material->position.x - 1, material->position.y + 1) && handler->get_material(material->position.x - 1, material->position.y + 1) == nullptr;
		can_move = left_clear && diag_left_clear;
		direction = 0;
	}

	if(!can_move)
	{
		return false;
	}

	if(direction == 0)
	{
		vector2 new_pos {material->position.x - 1, material->position.y + 1};
		handler->move_list.push_back({{material->position.x, material->position.y}, {new_pos.x, new_pos.y}});
		return true;
	}
	else if(direction == 1)
	{
		vector2 new_pos {material->position.x + 1, material->position.y + 1};
		handler->move_list.push_back({{material->position.x, material->position.y}, {new_pos.x, new_pos.y}});
		return true;
	}
	return false;
}


void Elements::update_sand(ELEMENT_UPDATE_ARGS)
{
	//Try to move
	if(update_down(material, dT) || update_side_down(material)){
		return;
	}

	//If not move then swap position with whatever below sand
	Material* displace_mat = handler->in_world(material->position.x, material->position.y + 1)? handler->get_material(material->position.x, material->position.y + 1): nullptr;
	Material* left_diag_mat = handler->in_world(material->position.x - 1, material->position.y + 1)? handler->get_material(material->position.x - 1, material->position.y + 1): nullptr;
	Material* right_diag_mat = handler->in_world(material->position.x + 1, material->position.y + 1)? handler->get_material(material->position.x + 1, material->position.y + 1): nullptr;

	if(displace_mat != nullptr && displace_mat->material != SAND && displace_mat->reaction & DISPLACIBLE)
	{
		handler->swap_list.push_back({material->position, displace_mat->position});
	}
	else if(left_diag_mat != nullptr && left_diag_mat->material != SAND && left_diag_mat->reaction & DISPLACIBLE && rand() % 2 == 1)
	{
		handler->swap_list.push_back({material->position, left_diag_mat->position});
	}
	else if(right_diag_mat != nullptr && right_diag_mat->material != SAND && right_diag_mat->reaction & DISPLACIBLE)
	{
		handler->swap_list.push_back({material->position, right_diag_mat->position});
	}
}

void Elements::update_smoke(ELEMENT_UPDATE_ARGS)
{
	if(update_up(material) || update_side_up(material)){
		return;
	}

	Material* displace_mat;
	if(handler->in_world(material->position.x, material->position.y - 1))
	{
		displace_mat = handler->get_material(material->position.x, material->position.y - 1);
	}
	else {
		return;
	}

	if(displace_mat != nullptr && displace_mat->material != SMOKE && displace_mat->property != STATIC)
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

	if(update_down(material, dT) || update_side_down(material) || update_side(material)){
		return;
	}
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
	if(up != nullptr && up->reaction & FLAMMABLE)
	{
		up->state = BURNING;
		mat_chunk = handler->get_chunk(up->position.x, up->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(down != nullptr && down->reaction & FLAMMABLE)
	{
		down->state = BURNING;
		mat_chunk = handler->get_chunk(down->position.x, down->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(up_left != nullptr && up_left->reaction & FLAMMABLE)
	{
		up_left->state = BURNING;
		mat_chunk = handler->get_chunk(up_left->position.x, up_left->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(up_right != nullptr && up_right->reaction & FLAMMABLE)
	{
		up_right->state = BURNING;
		mat_chunk = handler->get_chunk(up_right->position.x, up_right->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(down_right != nullptr && down_right->reaction & FLAMMABLE)
	{
		down_right->state = BURNING;
		mat_chunk = handler->get_chunk(down_right->position.x, down_right->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(down_left != nullptr && down_left->reaction & FLAMMABLE)
	{
		down_left->state = BURNING;
		mat_chunk = handler->get_chunk(down_left->position.x, down_left->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(left != nullptr && left->reaction & FLAMMABLE)
	{
		left->state = BURNING;
		mat_chunk = handler->get_chunk(left->position.x, left->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(right != nullptr && right->reaction & FLAMMABLE)
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

	if(up != nullptr && up->reaction & FLAMMABLE)
	{
		up->state = BURNING;
	}
	if(down != nullptr && down->reaction & FLAMMABLE)
	{
		down->state = BURNING;
	}
	if(up_left != nullptr && up_left->reaction & FLAMMABLE)
	{
		up_left->state = BURNING;
	}
	if(up_right != nullptr && up_right->reaction & FLAMMABLE)
	{
		up_right->state = BURNING;
	}
	if(down_right != nullptr && down_right->reaction & FLAMMABLE)
	{
		down_right->state = BURNING;
	}
	if(down_left != nullptr && down_left->reaction & FLAMMABLE)
	{
		down_left->state = BURNING;
	}
	if(left != nullptr && left->reaction & FLAMMABLE)
	{
		left->state = BURNING;
	}
	if(right != nullptr && right->reaction & FLAMMABLE)
	{
		right->state = BURNING;
	}


}

void Elements::update_acid(ELEMENT_UPDATE_ARGS)
{
	if(update_down(material, dT) || update_side_down(material) || update_side(material)){
		return;
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

	if(up != nullptr && up->reaction & ACID_DESTROY)
	{
		up->health -= 10;
		if(up->health <= 0)
		{
			handler->destroy_material(up);
		}
	}
	if(down != nullptr && down->reaction & ACID_DESTROY)
	{
		down->health -= 10;
		if(down->health <= 0)
		{
			handler->destroy_material(down);
		}
	}
	if(up_left != nullptr && up_left->reaction & ACID_DESTROY)
	{
		up_left->health -= 10;
		if(up_left->health <= 0)
		{
			handler->destroy_material(up_left);
		}
	}
	if(up_right != nullptr && up_right->reaction & ACID_DESTROY)
	{
		up_right->health -= 10;
		if(up_right->health <= 0)
		{
			handler->destroy_material(up_right);
		}
	}
	if(down_right != nullptr && down_right->reaction & ACID_DESTROY)
	{
		down_right->health -= 10;
		if(down_right->health <= 0)
		{
			handler->destroy_material(down_right);
		}
	}
	if(down_left != nullptr && down_left->reaction & ACID_DESTROY)
	{
		down_left->health -= 10;
		if(down_left->health <= 0)
		{
			handler->destroy_material(down_left);
		}
	}
	if(left != nullptr && left->reaction & ACID_DESTROY)
	{
		left->health -= 10;
		if(left->health <= 0)
		{
			handler->destroy_material(left);
		}
	}
	if(right != nullptr && right->reaction & ACID_DESTROY)
	{
		right->health -= 10;
		if(right->health <= 0)
		{
			handler->destroy_material(right);
		}
	}

}

void Elements::update_oil(ELEMENT_UPDATE_ARGS){
	if(update_down(material, dT) || update_side_down(material) || update_side(material));

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
	if(up != nullptr && up->reaction & FLAMMABLE)
	{
		up->state = BURNING;
		mat_chunk = handler->get_chunk(up->position.x, up->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(down != nullptr && down->reaction & FLAMMABLE)
	{
		down->state = BURNING;
		mat_chunk = handler->get_chunk(down->position.x, down->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(up_left != nullptr && up_left->reaction & FLAMMABLE)
	{
		up_left->state = BURNING;
		mat_chunk = handler->get_chunk(up_left->position.x, up_left->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(up_right != nullptr && up_right->reaction & FLAMMABLE)
	{
		up_right->state = BURNING;
		mat_chunk = handler->get_chunk(up_right->position.x, up_right->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(down_right != nullptr && down_right->reaction & FLAMMABLE)
	{
		down_right->state = BURNING;
		mat_chunk = handler->get_chunk(down_right->position.x, down_right->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(down_left != nullptr && down_left->reaction & FLAMMABLE)
	{
		down_left->state = BURNING;
		mat_chunk = handler->get_chunk(down_left->position.x, down_left->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(left != nullptr && left->reaction & FLAMMABLE)
	{
		left->state = BURNING;
		mat_chunk = handler->get_chunk(left->position.x, left->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}
	if(right != nullptr && right->reaction & FLAMMABLE)
	{
		right->state = BURNING;
		mat_chunk = handler->get_chunk(right->position.x, right->position.y);
		if(mat_chunk != nullptr && mat_chunk->asleep == 1)
		{
			mat_chunk->asleep = 0;
		}
	}

}

void Elements::init(ChunkHandler* handler){
	this->handler = handler;
}
