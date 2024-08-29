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

struct vector2
{
	int x {0};
	int y {0};

	bool operator<(const vector2& other) const {
		if(x != other.x)
		{
			return x < other.x;
		}
		return y < other.y;
	}
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
	vector2 position {0, 0};
	vector2 velocity {0, 0};
	MatType material = MatType::NONE;
	Properties property = Properties::STATIC;
};



#endif
