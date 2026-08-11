#include "Elements.hpp"
#include "ChunkHandler.hpp"
#include <string>
// #include <climits>
#include <iostream>

std::string Elements::print_vel(float x, float y)
{
	return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
}


void Elements::update_sand(const float dT, Material* material)
{
	solid_physics_update(material, dT, 0.1, 7.0, 5.0);

	vector2 rounded_velocity;
	round_velocity(&material->velocity, &rounded_velocity);

	if(rounded_velocity.x != 0 || rounded_velocity.y != 0)
	{
		handler->move_list.push_back(material);
	}
	else if(rounded_velocity.x == 0 && rounded_velocity.y == 0) {
		material->frames_rest ++;
	}

	if(material->frames_rest == 60)
	{
		material->frames_rest = 0;
		handler->set_asleep(material);
	}
}

void Elements::update_smoke(const float dT, Material* material)
{}


void Elements::update_water(const float dT, Material* material)
{}

void Elements::update_wood(const float dT, Material* material)
{}

void Elements::update_fire(const float dT, Material* material)
{}

void Elements::update_acid(const float dT, Material* material)
{}

void Elements::update_oil(const float dT, Material* material){}


void Elements::update_flammable_gas(const float dT, Material* material)
{}


void Elements::fluid_physics_update(Material* material, const float dT, const int fluid_spread, const float friction_constant)
{
}

void Elements::solid_physics_update(Material* material, const float dT, const float friction_const, const float air_resitance_const, const float collision_conserve)
{
	Neighbors neighbors = handler->get_neighbors(material->position, material->position);

	vector2 rounded_velocity;
	round_velocity(&material->velocity, &rounded_velocity);

	if(material->position.y == world_height - 1 && rounded_velocity.y > 0)
	{
		material->velocity.x += prob_gen(0.5f)? -material->velocity.y/collision_conserve: material->velocity.y/collision_conserve;
		material->velocity.y = 0;
	}
	if((material->position.x == 0 && rounded_velocity.x < 0) || (material->position.x == world_width - 1 && rounded_velocity.x > 0))
	{
		material->velocity.x = 0;
	}

	if(material->position.y == world_height - 1)
	{
		material->velocity.x = rounded_velocity.x < 0? material->velocity.x + friction_const * dT: material->velocity.x - friction_const * dT;
	}

	if(neighbors.org_down == nullptr && rounded_velocity.y > 0 && material->position.y != world_height - 1)
	{
		material->velocity.y += g_force * dT;
	}
	else if(neighbors.org_down == nullptr && rounded_velocity.y == 0 && material->position.y != world_height - 1)
	{
		material->velocity.y = 1.0f;
	}

	if(rounded_velocity.x > 0 && neighbors.org_down != nullptr && neighbors.org_dright != nullptr && neighbors.org_right == nullptr)
	{
		material->velocity.x = rounded_velocity.x < 0? material->velocity.x + friction_const * dT: material->velocity.x - friction_const * dT;
	}
	else if(rounded_velocity.x < 0 && neighbors.org_down != nullptr && neighbors.org_dleft != nullptr && neighbors.org_left == nullptr)
	{
		material->velocity.x = rounded_velocity.x < 0? material->velocity.x + friction_const * dT: material->velocity.x - friction_const * dT;
	}

	if(neighbors.org_down == nullptr && rounded_velocity.x != 0)
	{
		material->velocity.x = rounded_velocity.x < 0? material->velocity.x + air_resitance_const * dT: material->velocity.x - air_resitance_const * dT;
	}

	material->velocity.x = std::fabs(material->velocity.x) < 0.5f? 0: material->velocity.x;
}

void Elements::solid_collision_resolver(Material* material, Material* collided_material, vector2& new_pos, float inertia_chance, float collision_conserve)
{
	vector2 rounded_velocity;
	round_velocity(&material->velocity, &rounded_velocity);

	vector2 mat_pos = material->position;
	vector2 hit_mat_pos = collided_material->position;


	Neighbors neighbors = handler->get_neighbors(mat_pos, new_pos);

	if(new_pos.x == hit_mat_pos.x && new_pos.y < hit_mat_pos.y && rounded_velocity.x == 0 && rounded_velocity.y > 0)
	{
		bool cascade = inertia_chance == 1.0f? true: prob_gen(inertia_chance);
		if(cascade && neighbors.new_dleft == nullptr && neighbors.new_dright == nullptr && neighbors.new_left == nullptr && neighbors.new_right == nullptr)
		{
			new_pos.x = prob_gen(0.5f)? hit_mat_pos.x + 1: hit_mat_pos.x - 1;
			new_pos.y = hit_mat_pos.y;
			material->velocity.x = 0;
			material->velocity.y = 1.0f;
		}
		else if(cascade && (neighbors.new_dleft != nullptr || neighbors.new_left != nullptr) && (neighbors.new_right == nullptr && neighbors.new_dright == nullptr))
		{
			new_pos.x = hit_mat_pos.x + 1;
			new_pos.y = hit_mat_pos.y;
			material->velocity.x = 0;
			material->velocity.y = 1.0f;
		}
		else if(cascade && (neighbors.new_dleft == nullptr && neighbors.new_left == nullptr) && (neighbors.new_right != nullptr && neighbors.new_dright != nullptr))
		{
			new_pos.x = hit_mat_pos.x - 1;
			new_pos.y = hit_mat_pos.y;
			material->velocity.x = 0;
			material->velocity.y = 1.0f;
		}
		else if(neighbors.new_dleft != nullptr && neighbors.new_dright != nullptr && neighbors.new_left == nullptr && neighbors.new_right == nullptr)
		{
			material->velocity.x = prob_gen(0.5f)? -material->velocity.y/collision_conserve: material->velocity.y/collision_conserve;
			material->velocity.y = 0;
		}
		else if(neighbors.new_dleft != nullptr && neighbors.new_left == nullptr)
		{
			material->velocity.x = -material->velocity.y/collision_conserve;
			material->velocity.y = 0;
		}
		else if(neighbors.new_dright != nullptr && neighbors.new_right == nullptr)
		{
			material->velocity.x = material->velocity.y/collision_conserve;
			material->velocity.y = 0;
		}
		else if(neighbors.new_left != nullptr && neighbors.new_left != nullptr)
		{
			material->velocity.x = 0;
			material->velocity.y = 0;
		}
	}
	else if(rounded_velocity.x > 0 && rounded_velocity.y > 0 && new_pos.x == hit_mat_pos.x && new_pos.y > hit_mat_pos.y)
	{
		bool cascade = prob_gen(inertia_chance);
		if(cascade && neighbors.new_right == nullptr && neighbors.new_dright == nullptr)
		{
			new_pos.x = hit_mat_pos.x + 1;
			new_pos.y = hit_mat_pos.y;
			material->velocity.x = 0;
			material->velocity.y = 1.0f;
		}
		else if(cascade && neighbors.new_right != nullptr && neighbors.new_left == nullptr && neighbors.new_dleft == nullptr)
		{
			new_pos.x = hit_mat_pos.x - 1;
			new_pos.y = hit_mat_pos.y;
			material->velocity.x = 0;
			material->velocity.y = 1.0f;
		}
		else if(neighbors.new_right == nullptr)
		{
			material->velocity.x += material->velocity.y/collision_conserve;
			material->velocity.y = 0;
		}
		else if(neighbors.new_right != nullptr)
		{
			material->velocity.x = 0;
			material->velocity.y = 0;
		}
	}
	else if(rounded_velocity.x < 0 && rounded_velocity.y > 0 && new_pos.x == hit_mat_pos.x && new_pos.y > hit_mat_pos.y)
	{
		bool cascade = prob_gen(inertia_chance);
		if(cascade && neighbors.new_left == nullptr && neighbors.new_dleft == nullptr)
		{
			new_pos.x = hit_mat_pos.x - 1;
			new_pos.y = hit_mat_pos.y;
			material->velocity.x = 0;
			material->velocity.y = 1.0f;
		}
		else if(cascade && neighbors.new_left != nullptr && neighbors.new_right == nullptr && neighbors.new_dright == nullptr)
		{
			new_pos.x = hit_mat_pos.x + 1;
			new_pos.y = hit_mat_pos.y;
			material->velocity.x = 0;
			material->velocity.y = 1.0f;
		}
		else if(neighbors.new_left == nullptr)
		{
			material->velocity.x += material->velocity.y/collision_conserve;
			material->velocity.y = 0;
		}
		else if(neighbors.new_left != nullptr)
		{
			material->velocity.x = 0;
			material->velocity.y = 0;
		}
	}
	else if(rounded_velocity.x != 0 && rounded_velocity.y > 0 && (new_pos.x > hit_mat_pos.x || new_pos.x < hit_mat_pos.x) && new_pos.y < hit_mat_pos.y)
	{
		material->velocity.x = 0;
	}
	else if(rounded_velocity.x != 0 && rounded_velocity.y > 0 && (new_pos.x > hit_mat_pos.x || new_pos.x < hit_mat_pos.x) && new_pos.y == hit_mat_pos.y)
	{
		material->velocity.x = 0;
	}
	else if(rounded_velocity.x != 0 && rounded_velocity.y == 0 && (new_pos.x > hit_mat_pos.x || new_pos.x < hit_mat_pos.x) && new_pos.y == hit_mat_pos.y)
	{
		material->velocity.x = 0;
	}

}

vector2 Elements::pos_update(vector2* pos, fvector2* velocity, Material** colliding_material) //Brenesham algorithm adapted to my grid world
{
	int delta_x = velocity->x < 0? static_cast<int>(velocity->x - 0.5f):static_cast<int>(velocity->x + 0.5f); 
	int delta_y = velocity->y < 0? static_cast<int>(velocity->y - 0.5f):static_cast<int>(velocity->y + 0.5f);

	int x_mod = velocity->x < 0? -1: 1;
	int y_mod = velocity->y < 0? -1: 1;

	int short_side = std::min(std::abs(delta_x), std::abs(delta_y));
	int long_side = std::max(std::abs(delta_x), std::abs(delta_y));

	float error = 0;
	float error_step = (short_side == 0 || long_side == 0)? 0: static_cast<float>(short_side)/static_cast<float>(long_side);

	int new_x = pos->x, new_y = pos->y;
	bool diag = false, in_world = true, x_long = std::abs(delta_x) == long_side;

	for(int i = 1; i < long_side + 1; i ++)
	{
		new_x = x_long? new_x + x_mod: new_x;
		new_y = !x_long? new_y + y_mod: new_y;

		error += error_step;

		if(error_step > 0 && (error > 0.5f || within_eps(error, 0.5f, 0.01)))
		{
			new_x = !x_long? new_x + x_mod: new_x;
			new_y = x_long? new_y + y_mod: new_y;
			error -= 0.5f;
			diag = true;
		}
		else if(error_step < 0 && (error < -0.5f || within_eps(error, -0.5f, 0.01)))
		{
			new_x = !x_long? new_x + x_mod: new_x;
			new_y = x_long? new_y + y_mod: new_y;
			error += 0.5f;
			diag = true;
		}

		if(!(in_world = handler->in_world(new_x, new_y)) || (*colliding_material = handler->get_material(new_x, new_y)))
		{
			break;
		}

		diag = false;
	}

	if(diag && (*colliding_material || !in_world))
	{
		new_x -= x_mod;
		new_y -= (y_mod);
	}
	else if(!diag && x_long && (*colliding_material || !in_world))
	{
		new_x += -(x_mod);
	}
	else if(!diag && !x_long && (*colliding_material || !in_world))
	{
		new_y += -(y_mod);
	}
	
	assert(handler->in_world(new_x, new_y));

	return {new_x, new_y};
}

void Elements::init(ChunkHandler* handler, int wW, int wH){
	this->handler = handler;
	world_width = wW;
	world_height = wH;
}
