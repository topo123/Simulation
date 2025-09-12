#ifndef ELEMENTS_H
#define ELEMENTS_H

#include "Material.hpp"
#include <random>
#define ELEMENT_UPDATE_ARGS Material* material, const float dT
class ChunkHandler;

class Elements{

public:
	std::random_device rd;
	std::mt19937 gen;
	std::bernoulli_distribution random_generator;

	ChunkHandler* handler;
	Material* dummy_material;
	int world_height;
	int world_width;

	const float g_force = 7.5f;
	const int fast_liquid_spread = 5;

	std::string print_vel(float x, float y);
	inline void round_velocity(fvector2* actual_velocity, vector2* rounded_velocity)
	{
		rounded_velocity->x = actual_velocity->x >= 0? static_cast<int>(actual_velocity->x + 0.5f): static_cast<int>(actual_velocity->x - 0.5f);
		rounded_velocity->y = actual_velocity->y >= 0? static_cast<int>(actual_velocity->y + 0.5f): static_cast<int>(actual_velocity->y - 0.5f);
	}
	inline void clamp(float& value, float episolon, float clamp_value)
	{
		value = std::abs(value - clamp_value) < episolon? clamp_value: value;
	}
	inline bool prob_gen(float chance)
	{
		random_generator = std::bernoulli_distribution(chance);
		return random_generator(gen);
	};
	inline bool within_eps(float value, float compare_value, float episolon)
	{
		return std::abs(value - compare_value) < episolon;
	};

	vector2 pos_update(vector2* pos, fvector2* velocity, Material** colliding_material);
	vector2 solid_physics_update(ELEMENT_UPDATE_ARGS, float inertia_chance, float friction_constant);
	vector2 liquid_physics_update(ELEMENT_UPDATE_ARGS);
	vector2 gas_physics_update(ELEMENT_UPDATE_ARGS);

	void update_sand(ELEMENT_UPDATE_ARGS);
	void update_water(ELEMENT_UPDATE_ARGS);
	void update_acid(ELEMENT_UPDATE_ARGS);
	void update_wood(ELEMENT_UPDATE_ARGS);
	void update_fire(ELEMENT_UPDATE_ARGS);
	void update_smoke(ELEMENT_UPDATE_ARGS);
	void update_stone(ELEMENT_UPDATE_ARGS);
	void update_oil(ELEMENT_UPDATE_ARGS);
	void update_flammable_gas(ELEMENT_UPDATE_ARGS);

	void init(ChunkHandler* handler, int wW, int wH);
};


#endif
