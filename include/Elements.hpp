#ifndef ELEMENTS_H
#define ELEMENTS_H

#include "Material.hpp"
#define ELEMENT_UPDATE_ARGS Material* material, const float dT
class ChunkHandler;

class Elements{

public:
	ChunkHandler* handler;
	int world_height;
	int world_width;
	const static int g_force = 2;
	const static int fast_liquid_spread = 5;

	bool update_down(ELEMENT_UPDATE_ARGS);
	bool update_side(Material* material);
	bool update_side_down(Material* material);
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

	void init(ChunkHandler* handler);
};

#endif
