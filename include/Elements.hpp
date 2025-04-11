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

	ChunkHandler* handler;
	int world_height;
	int world_width;

	const float g_force = 10.0f;
	const int fast_liquid_spread = 5;

	vector2 velocity_update(vector2* pos, fvector2* velocity);

	bool solid_physics_update(ELEMENT_UPDATE_ARGS);
	bool update_side(Material* material);
	bool update_up(Material* material);
	bool update_side_up(Material* material);

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
