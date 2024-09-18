#include <World.hpp>
#include <cassert>

void World::init_world(int cW, int cH, int wW, int wH)
{
	world_width = wW;
	world_height = wH;
	
	render.initRenderData();
	handler.init_chunk_handler(cW, cH, wW, wH);
}

void World::create_materials(int center_x, int center_y, int width, int height, MatType type, PoolArena* arena)
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
		lX = 0;
	}
	if(rX > world_width)
	{
		num_cols -= rX - world_width;
		rX = world_width - 1;
	}
	if(tY < 0)
	{
		num_rows += tY;
		tY = 0;
	}
	if(bY > world_height)
	{
		num_rows -= bY - world_height;
		bY = world_height - 1;
	}

	int prev_x = rX;

	std::vector<Material*> materials;
	materials.resize(num_rows * num_cols, nullptr);

	int counter = 0;
	for(size_t i = 0; i < num_rows * num_cols; i ++)
	{
		materials[i] = static_cast<Material*>(allocate(arena));
		vector2 pos;
		pos.x = rX;
		pos.y = bY;

		set_material_properties(materials[i], type, &pos);
		rX --;

		if(rX == lX - 1)
		{
			counter ++;
			rX = prev_x;
			bY --;
		}
	}

	for(size_t i = 0; i < materials.size(); i ++)
	{
		assert(materials[i] != nullptr);
	}
	handler.add_materials(materials, arena); 
}


void World::set_material_properties(Material* material, MatType type, vector2* pos)
{
	material->material = type;
	material->position.x = pos->x;
	material->position.y = pos->y;
	material->velocity.x = 0;
	material->velocity.y = 1;
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
	else if(type == STONE)
	{
		material->tex_offset = tex_coords.STONE;
		material->property = static_cast<Properties>(STATIC);
	}
}


void World::update_world()
{
	for(size_t i = 0; i < handler.iter_chunks.size(); i ++)
	{
		ChunkHandler::Chunk* chunk = handler.iter_chunks[i];
		assert(handler.chunks.find(chunk->coords) != handler.chunks.end());
		handler.update_chunk(chunk);
		//std::sort(chunk->moves.begin(), chunk->moves.end(), [](ChunkHandler::Move& a, ChunkHandler::Move& b){return a.old_pos.x < b.old_pos.x;});
		handler.commit_changes(chunk);
		if(chunk->num_materials == 0)
		{
			handler.iter_chunks.erase(handler.iter_chunks.begin() + i);
			handler.chunks.erase(chunk->coords);
			chunk->update_list.clear();
			chunk->materials.clear();
			delete chunk;
		}
	}
}

void World::draw_world()
{
	
	for(ChunkHandler::Chunk* chunk: handler.iter_chunks)
	{
		vector2 end_coords;
		end_coords.x = (chunk->coords.x * handler.chunk_width) + handler.chunk_width;
		end_coords.y = (chunk->coords.y * handler.chunk_height) + handler.chunk_height;
		vector2 start_coords {chunk->coords.x * handler.chunk_width, chunk->coords.y * handler.chunk_height};
		handler.draw_chunk(chunk, render);
	}
}
