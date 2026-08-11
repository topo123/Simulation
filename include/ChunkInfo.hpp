#pragma once

#include "Material.hpp"
#include <set>

struct Move
{
	vector2 old_pos;
	vector2 new_pos;
};

struct Animation
{
	float color_change;
	int frames;
};

struct Chunk
{
	std::set<Material*> active_list;
	std::set<Material*> asleep_list;
	vector2 upper_rect_coords{-1,-1};
	vector2 lower_rect_coords{-1, -1};
	vector2 coords{0, 0};
	Material** grid;
	int num_materials {0};
};



