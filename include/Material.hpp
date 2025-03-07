#ifndef MATERIAL_H
#define MATERIAL_H
#include <functional>

#define NUM_COLORS 10.0f

enum Properties{
	STATIC = 0b00000000,
	DOWN = 0b00000001,
	DOWN_SIDE = 0b00000010,
	SIDE = 0b00000100,
	UP = 0b00001000,
	UP_SIDE = 0b00010000,
	SHORT_LIVED = 0b00100000
};

enum ReactionProperties{
	FLAMMABLE = 0b01,
	STABLE = 0b00
};



struct fvector2{
	float x{0};
	float y{0};
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

	bool operator==(const vector2& other) const
	{
		return other.x == x && other.y == y;
	}
};


struct vector_hash
{
	std::size_t operator()(const vector2& vec) const {
		return (std::hash<int>()(vec.x)) ^ (std::hash<int>()(vec.y));
	}
};

enum MatType
{
	NONE,
	SAND,
	WATER,
	SMOKE,
	FIRE,
	WOOD,
	STONE,
	ACID,
	NITRO
};

struct MatTexCoords
{
	const float SAND {0.0f};
	const float WATER = {1.0f/NUM_COLORS};
	const float STONE = {2.0f/NUM_COLORS};
	const float ACID = {3.0f/NUM_COLORS};
	const float SMOKE = {4.0f/NUM_COLORS};
	const float WOOD = {5.0f/NUM_COLORS};
	const float FIRE = {6.0f/NUM_COLORS};
	const float FIRE_CHANGE_COLOR = {7.0f/NUM_COLORS};
	const float DIRTY_DEBUG = {8.0f/NUM_COLORS};
	const float GRID_DEBUG = {9.0f/NUM_COLORS};
};

struct Material
{
	vector2 position {0, 0};
	fvector2 velocity {0, 0};
	MatType material = MatType::NONE;
	Properties property = Properties::STATIC;
	ReactionProperties reaction = ReactionProperties::STABLE;
	float tex_offset;
	int health;
};



#endif
