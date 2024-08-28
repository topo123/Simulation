#include <World.hpp>
#include <cassert>
#include <iostream>

void World::init_world(uint16_t cW, uint16_t cH, uint16_t wW, uint16_t wH)
{
	world_width = wW;
	world_height = wH;
	
	render.initRenderData();
	handler.init_chunk_handler(cW, cH, wW, wH);
}

void World::create_materials(uint16_t center_x, uint16_t center_y, uint16_t width, uint16_t height, MatType type, PoolArena* arena)
{

	if(width % 2 == 0 || height % 2 == 0)
	{
		return;
	}

	if(center_x < 0 || center_x > 800 || center_y < 0 || center_y > 600)
	{
		return;
	}

	int num_cols = width;
	int num_rows = height;

	int half_x = width/2;
	int half_y = height/2;

	int lX = center_x - half_x;
	int rX = center_x + half_x;
	int tY = center_y - half_y;
	int bY = center_y + half_y;

	if(lX < 0)
	{
		num_cols += lX;
	}
	if(rX > 800)
	{
		num_cols -= rX - 800;
	}
	if(tY < 0)
	{
		num_rows += tY;
	}
	if(bY > 600)
	{
		num_rows -= bY - 600 ;
	}

	int prev_x = rX;
	int index = 0;

	Material** materials = new Material*[num_cols * num_rows];
	for(size_t i = 0; i < num_rows; i ++)
	{
		for(size_t j = 0; j < num_cols; j ++)
		{
			materials[index] = static_cast<Material*>(allocate(arena));
			set_material_properties(materials[index], type, std::make_pair(rX, bY));
			rX --;
			index ++;
		}
		rX = prev_x;
		bY --;
	}

	for(size_t i = 0; i < num_cols * num_rows; i ++)
	{
		assert(materials[i] != nullptr);
	}

	handler.add_materials(materials, num_rows * num_cols, arena); 
}


void World::set_material_properties(Material* material, MatType type, std::pair<uint16_t, uint16_t> pos)
{
	material->material = type;
	material->position = pos;
	material->velocity.second = 1;
	if(type == WATER)
	{
		material->tex_offset = tex_coords.WATER;
		material->property = static_cast<Properties>(DOWN_SIDE + DOWN + SIDE);
	}
	else if(type == SAND)
	{
		material->tex_offset = tex_coords.SAND;
		material->property = static_cast<Properties>(DOWN + DOWN_SIDE);
	}
}

void World::update_world()
{
	for(ChunkHandler::Chunk* chunk: handler.iter_chunks)
	{
		handler.update_chunk(chunk);
	}
}

void World::draw_world()
{
	for(ChunkHandler::Chunk* chunk: handler.iter_chunks)
	{
		handler.draw_chunk(chunk, render);
	}
}
