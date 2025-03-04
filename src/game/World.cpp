#include <World.hpp>
#include <cassert>

void World::init_world(int cW, int cH, int wW, int wH, PoolArena* material_arena)
{
	world_width = wW;
	world_height = wH;
	
	arena = material_arena;
	render.initRenderData();
	handler.init_chunk_handler(cW, cH, wW, wH, material_arena);
}

void World::create_materials(int center_x, int center_y, int width, int height, MatType type)
{

	if(width % 2 == 0 || height % 2 == 0)
	{
		return;
	}

	if(center_x < 0 || center_x > world_width || center_y < 0 || center_y > world_height)
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
	vector2 pos;
	for(size_t i = 0; i < num_rows * num_cols; i ++)
	{
		materials[i] = static_cast<Material*>(allocate(arena));
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

	handler.add_materials(materials); 
}

void World::delete_materials(int center_x, int center_y, int width, int height)
{
	if(width % 2 == 0 || height % 2 == 0)
	{
		return;
	}

	if(center_x < 0 || center_x > world_width || center_y < 0 || center_y > world_height)
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

	vector2 pos;
	for(size_t i = 0; i < num_rows * num_cols; i ++)
	{
		pos.x = rX;
		pos.y = bY;

		if(!handler.in_world(rX, bY)){
			rX --;

			if(rX == lX - 1)
			{
				rX = prev_x;
				bY --;
			}
			continue;
		}

		Material* material = handler.get_material(pos.x, pos.y);

		if(material != nullptr)
		{
			handler.destroy_material(material);
		}

		rX --;

		if(rX == lX - 1)
		{
			rX = prev_x;
			bY --;
		}
	}


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
		material->health = 50;
		material->tex_offset = tex_coords.WATER;
		material->property = static_cast<Properties>(DOWN_SIDE + DOWN + SIDE);
	}
	else if(type == SAND)
	{
		material->health = 500;
		material->tex_offset = tex_coords.SAND;
		material->property = static_cast<Properties>(DOWN + DOWN_SIDE);
	}
	else if(type == STONE)
	{
		material->health = 500;
		material->tex_offset = tex_coords.STONE;
		material->property = static_cast<Properties>(STATIC);
	}
	else if(type == ACID)
	{
		material->tex_offset = tex_coords.ACID;
		material->health = 200;
		material->property = static_cast<Properties>(DOWN_SIDE + DOWN + SIDE);
	}
	else if(type == SMOKE)
	{
		material->tex_offset = tex_coords.SMOKE;
		material->health = 200;
		material->property = static_cast<Properties>(UP_SIDE + UP + SIDE);
	}
}


void World::update_world()
{
	ChunkHandler::Chunk* chunk;
	ChunkHandler::Chunk* delete_chunk; 
	for(size_t i = 0; i < handler.iter_chunks.size(); i ++)
	{
		chunk = handler.iter_chunks[i];
		assert(handler.chunks.find(chunk->coords) != handler.chunks.end());
		handler.update_chunk(chunk);
		handler.commit_changes(chunk);
		if(chunk->num_materials == 0){
			handler.delete_chunks.push_back(chunk);
			handler.chunks.erase(chunk->coords);
			handler.iter_chunks[i] = handler.iter_chunks.back();
			handler.iter_chunks.pop_back();
			i --;
		}
	}

	for(size_t i = 0; i < handler.delete_chunks.size(); i ++){
		delete_chunk = handler.delete_chunks[i];
		delete_chunk->update_list.clear();
		delete_chunk->materials.clear();
		delete delete_chunk;
	}

	for(size_t i = 0; i < handler.add_materials_list.size(); i ++){
		handler.iter_chunks.push_back(handler.add_materials_list[i]);
	}
	handler.add_materials_list.clear();
	handler.delete_chunks.clear();
}

void World::draw_world()
{

	for(ChunkHandler::Chunk* chunk: handler.iter_chunks)
	{
		handler.draw_chunk(chunk, &render);
	}
}


