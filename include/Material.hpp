#ifndef MATERIAL_H
#define MATERIAL_H
#include <functional>
#include <cstdint>

#define NUM_COLORS 12.0f


enum Properties{
	STATIC = 0b00000000,
	DOWN = 0b00000001,
	DOWN_SIDE = 0b00000010,
	SIDE = 0b00000100,
	UP = 0b00001000,
	UP_SIDE = 0b00010000,
	SHORT_LIVED = 0b00100000,
	PHYSICS = 0b01000000
};

enum ReactionDirection{
	RDOWN = 0b10000,
	RUP = 0b01000,
	RSIDE = 0b00100,
	AROUND = 0b00010,
	RNONE = 0b00001
};

enum ReactionProperties{
	ACID_DESTROY = 0b1000,
	DISPLACIBLE = 0b0100,
	FLAMMABLE = 0b0010,
	STABLE = 0b0000
};

struct fvector2{
	float x{0};
	float y{0};

	bool operator==(const fvector2& other) const
	{
		return x == other.x && y == other.x;
	}
	bool operator!=(const fvector2& other)const
	{
		return other.x != x || other.y != y;
	}
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

	bool operator!=(const vector2& other)const
	{
		return other.x != x || other.y != y;
	}
};


struct vector_hash
{
	std::size_t operator()(const vector2& vec) const {
		return (std::hash<int>()(vec.x)) ^ (std::hash<int>()(vec.y));
	}
};

enum MatType: uint8_t
{
	SAND = 0,
	WATER = 1,
	SMOKE = 2,
	FIRE = 3,
	WOOD = 4,
	STONE = 5,
	ACID = 6,
	NITRO = 7,
	OIL = 8,
	FLAMMABLE_GAS = 9,
	NONE = 10
};

struct MatTexCoords
{
	const float SAND {0.5f/NUM_COLORS};
	const float WATER = {(1.0f + 0.5f)/NUM_COLORS};
	const float STONE = {(2.0f + 0.5f)/NUM_COLORS};
	const float ACID = {(3.0f + 0.5f)/NUM_COLORS};
	const float SMOKE = {(4.0f + 0.5f)/NUM_COLORS};
	const float WOOD = {(5.0f + 0.5f)/NUM_COLORS};
	const float FIRE = {(6.0f + 0.5f)/NUM_COLORS};
	const float FIRE_CHANGE_COLOR = {(7.0f + 0.5f)/NUM_COLORS};
	const float OIL = {(8.0f + 0.5f)/NUM_COLORS};
	const float FL_GAS = {(9.0f + 0.5f)/NUM_COLORS};
	const float DIRTY_DEBUG = {(10.0f + 0.5f)/NUM_COLORS};
	const float GRID_DEBUG = {(11.0f + 0.5f)/NUM_COLORS};
};

struct serialized_material{
	int x_pos;
	int y_pos;
	float x_vel;
	float y_vel;
	uint32_t material_type;
};
enum MaterialState{
	SOLID,
	LIQUID,
	GAS,
	ENERGY
};

enum State{
	BURNING,
	NORMAL,
	CONDUCTING
};

enum PhysicsState{
	FREE_FALLING, 
	SLIDING, 
	RESTING
};

struct MaterialProps
{
	Properties property;
	ReactionProperties reaction;
	ReactionDirection reaction_direction;
	int health;
	float tex_offset;
	MaterialState mat_state;
};

struct Material
{
	vector2 position {0, 0};
	fvector2 velocity {0, 0};
	MatType material = MatType::NONE;
	State state = State::NORMAL;
	PhysicsState phys_state = PhysicsState::FREE_FALLING;
	unsigned int frames_rest;
	float tex_offset;
	int health;
	int chunk_index;
};



#endif
