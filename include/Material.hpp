#ifndef MATERIAL_H
#define MATERIAL_H
#include <functional>
#include <cstdint>

#define NUM_COLORS 12.0f

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

enum class MatType: uint32_t
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

enum class MatterState: int32_t {
	STATIC_SOLID,
	DYNAMIC_SOLID,
	LIQUID,
	GAS,
	ENERGY
};

enum class ReactionState: int32_t {
	BURNING,
	NORMAL,
	CONDUCTING
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
	MatType mat_id;
};

struct MaterialProps
{
	MatterState mat_state;
	ReactionProperties reaction;
	int health;
	float tex_offset;
};

struct Material
{
	vector2 position {0, 0};
	fvector2 velocity {0, 0};
	MatType mat_id = MatType::NONE;
	ReactionState state = ReactionState::NORMAL;
	float tex_offset;
	int health;
	int frames_rest {0};
};

struct Neighbors
{
	Material* org_up;
	Material* org_down;
	Material* org_right;
	Material* org_left;
	Material* org_dleft;
	Material* org_dright;

	Material* new_down;
	Material* new_right;
	Material* new_left;
	Material* new_dleft;
	Material* new_dright;
};
#endif
