#pragma once

#include "Material.hpp"


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

	struct PhysicsMaterialMoveData
	{
		Material* org_material;
		Material* hit_material;
		vector2 new_pos;
	};

	struct Chunk
	{
		std::vector<Material*> active_list;
		std::vector<Material*> asleep_list;
		std::vector<PhysicsMaterialMoveData> move_data;
		vector2 upper_rect_coords{-1,-1};
		vector2 lower_rect_coords{-1, -1};
		vector2 coords{0, 0};
		Material** grid;
		int num_materials {0};
	};


