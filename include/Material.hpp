#ifndef MATERIAL_H
#define MATERIAL_H
#include <cstdint>
#include <utility>

#define NUM_MATERIALS 2.0f

enum Properties{
	STATIC = 0b00000000,
	DOWN = 0b00000001,
	DOWN_SIDE = 0b00000010,
	SIDE = 0b00000100
};

enum MatType
{
	NONE,
	SAND,
	WATER,
	FIRE,
	WOOD,
	ACID,
	NITRO
};

struct MatTexCoords
{
	const float SAND {0.0f};
	const float WATER = {1.0f/NUM_MATERIALS};
};

struct Material
{
	float tex_offset;
	std::pair<uint16_t, uint16_t> position;
	std::pair<uint16_t, uint16_t> velocity;
	MatType material = MatType::NONE;
	Properties property = Properties::STATIC;
};



#endif
