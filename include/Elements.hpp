#ifndef ELEMENTS_H

#define ELEMENTS_H

#include "Material.hpp"
#include "ChunkInfo.hpp"
#include <random>

class ChunkHandler;

class Elements{

public:
	std::random_device rd;
	std::mt19937 gen;
	std::bernoulli_distribution random_generator;

	ChunkHandler* handler;
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
	inline void clamp(float& value, const float max_value, const float min_value)
	{
		value = value > max_value? max_value: value;
		value = value < min_value? min_value: value;
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

	void solid_physics_update(Material* material, const float dT, const float friction_const, const float air_resistance_const, const float collision_conserve);
	void solid_collision_resolver(Material* material, Material* collided_material, vector2& new_pos, const float inertia_chance, const float collision_conserve);
	void fluid_physics_update(Material* material, const float dT, const int fluid_spread, const float air_resistance);
	void gas_physics_update(Material* material, const float dT);

	void update_sand(const float dT, Material* material);
	void update_water(const float dT, Material* material);
	void update_acid(const float dT, Material* material);
	void update_wood(const float dT, Material* material);
	void update_fire(const float dT, Material* material);
	void update_smoke(const float dT, Material* material);
	void update_stone(const float dT, Material* material);
	void update_oil(const float dT, Material* material);
	void update_flammable_gas(const float dT, Material* material);

	void init(ChunkHandler* handler, int wW, int wH);
};


#endif
