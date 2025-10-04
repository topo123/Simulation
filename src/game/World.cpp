#include <World.hpp>
#include <unordered_set>
#include <cassert>
#include <iostream>
#include <fstream>

void World::init_world(int cW, int cH, int wW, int wH, PoolArena* material_arena)
{
	world_width = wW;
	world_height = wH;

	arena = material_arena;
	std::cout << "Initializing Render data\n";
	render.initRenderData();
	handler.init_chunk_handler(cW, cH, wW, wH, material_arena);
}


void World::create_materials(const vector2& old_center, const vector2& new_center, const vector2& draw_size, const MatType type)
{

	if(draw_size.x % 2 == 0 || draw_size.y % 2 == 0)
	{
		return;
	}

	if(new_center.x < 0 || new_center.x > world_width || new_center.y < 0 || new_center.y > world_height)
	{
		return;
	}


	std::vector<Material*> materials;

	vector2 center = old_center;

	float delta_y = new_center.y - old_center.y;
	float delta_x = new_center.x - old_center.x;

	if(delta_y == 0 && delta_x == 0)
	{
		spawn_materials(center, draw_size, materials, type);
		return;
	}

	float slope;
	float intercept;

	bool horz = delta_x != 0 && delta_y == 0;
	bool vert = delta_x == 0 && delta_y != 0;

	while(horz)
	{
		spawn_materials(center, draw_size, materials, type);
		center.x += draw_size.x;

		if(center.x >= new_center.x && delta_x > 0)
		{
			spawn_materials(center, draw_size, materials, type);
			return;
		}
		else if(center.x <= new_center.x && delta_x < 0)
		{
			spawn_materials(center, draw_size, materials, type);
			return;
		}
	}

	while(vert)
	{
		spawn_materials(center, draw_size, materials, type);
		center.y += draw_size.y;

		if(center.y >= new_center.y && delta_y > 0)
		{
			spawn_materials(center, draw_size, materials, type);
			return;
		}
		else if(center.y <= new_center.y && delta_y < 0)
		{
			spawn_materials(center, draw_size, materials, type);
			return;
		}
	}


	bool should_terminate = false;

	if(delta_y > delta_x)
	{
		slope = delta_x/delta_y;
		intercept = (old_center.y/slope) - old_center.x;
	}
	else{
		slope = delta_y/delta_x;
		intercept = old_center.y - slope * old_center.x;
	}


	while(delta_y > delta_x && !should_terminate)
	{
		spawn_materials(center, draw_size, materials, type);

		center.y = delta_y > 0? center.y + draw_size.y: center.y - draw_size.y;  
		center.x = slope * center.y - intercept;

		if((center.y > new_center.y && delta_y > 0) || (center.y < new_center.y && delta_y < 0))
		{
			center.y = new_center.y;
			center.x = slope * center.y - intercept;
			spawn_materials(center, draw_size, materials, type);
			should_terminate = true;
		}
	}


	while(delta_y < delta_x && !should_terminate)
	{
		spawn_materials(center, draw_size, materials, type);

		center.x = delta_x > 0? center.x + draw_size.x: center.x - draw_size.x;
		center.y = slope * center.x + intercept;

		if((center.x > new_center.x && delta_x > 0) && (center.x < new_center.x && delta_x < 0))
		{
			center.x = new_center.x;
			center.y = slope * center.x + intercept;
			spawn_materials(center, draw_size, materials, type);
			should_terminate = true;
		}
	}
}

void World::flame_brush(const int num_materials, int lower_x, int lower_y, int upper_x, int upper_y, std::vector<Material*>& materials)
{
	int counter = 0;
	int flame_counter = 0;
	vector2 pos;
	std::unordered_set<vector2, vector_hash> filled_pos;

	while(flame_counter < num_materials)
	{
		pos.x = rand() % (lower_x - upper_x) + upper_x;
		pos.y = rand() % (lower_y - upper_y) + upper_y;

		if(rand() % 2 == 1 && filled_pos.find(pos) == filled_pos.end()){
			materials.push_back(static_cast<Material*>(allocate(arena)));
			handler.set_material_properties(materials.back(), FIRE, &pos);
			counter ++;
			flame_counter ++;
			filled_pos.insert(pos);
		}
	}

}

void World::regular_brush(const int num_materials, int lower_x, int lower_y, int upper_x, MatType type, std::vector<Material*>& materials)
{
	int prev_x = lower_x;
	vector2 pos;

	for(size_t i = 0; i < num_materials; i ++)
	{
		pos.x = lower_x;
		pos.y = lower_y;

		if(!handler.get_material(pos.x, pos.y) && handler.in_world(pos.x, pos.y))
		{
			materials.push_back(static_cast<Material*>(allocate(arena)));
			handler.set_material_properties(materials.back(), type, &pos);
		}

		lower_x --;

		if(lower_x == upper_x - 1)
		{
			lower_x = prev_x;
			lower_y --;
		}
	}

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





void World::update_world(const float dT)
{
	ChunkHandler::Chunk* chunk;
	ChunkHandler::Chunk* delete_chunk; 
	for(size_t i = 0; i < handler.iter_chunks.size(); i ++)
	{
		chunk = handler.iter_chunks[i];
		assert(handler.chunks.find(chunk->coords) != handler.chunks.end());
		handler.update_chunk(chunk, dT);
		handler.commit_changes();
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
		delete[] delete_chunk->materials;
		delete delete_chunk;
	}

	for(size_t i = 0; i < handler.add_materials_list.size(); i ++){
		handler.iter_chunks.push_back(handler.add_materials_list[i]);
	}
	handler.add_materials_list.clear();
	handler.delete_chunks.clear();
}

void World::draw_world(bool debug_mode)
{

	for(ChunkHandler::Chunk* chunk: handler.iter_chunks)
	{
		handler.draw_chunk_to_texture(chunk, &render, debug_mode);
	}
	render.screen_render();
}

void World::load_world(std::string name)
{
	std::ifstream sim_read(name, std::ios::binary);
	std::vector<Material*> materials;

	while(sim_read.peek() != EOF)
	{
		Material* material = static_cast<Material*>(allocate(arena));
		sim_read.read(reinterpret_cast<char*>(&material->position.x), sizeof(int));
		sim_read.read(reinterpret_cast<char*>(&material->position.y), sizeof(int));
		sim_read.read(reinterpret_cast<char*>(&material->velocity.x), sizeof(float));
		sim_read.read(reinterpret_cast<char*>(&material->velocity.y), sizeof(float));
		sim_read.read(reinterpret_cast<char*>(&material->material), sizeof(uint32_t));
		handler.set_material_properties(material, material->material, &material->position);
		materials.push_back(material);
	}

	handler.add_materials(materials);
}


void World::save_world(std::string name)
{
	int counter = 0;
	for(ChunkHandler::Chunk* chunk: handler.iter_chunks)
	{
		for(Material* material: chunk->update_list)
		{
			save_materials.push_back({material->position.x, material->position.y, material->velocity.x, material->velocity.y, material->material});
			counter ++;
		}
	}

	std::cout << "Saved " << counter << " materials\n";

	std::ofstream sim_write(name, std::ios::binary);
	for(serialized_material mat: save_materials)
	{
		sim_write.write(reinterpret_cast<const char*>(&mat.x_pos), sizeof(int));
		sim_write.write(reinterpret_cast<const char*>(&mat.y_pos), sizeof(int));
		sim_write.write(reinterpret_cast<const char*>(&mat.x_vel), sizeof(float));
		sim_write.write(reinterpret_cast<const char*>(&mat.y_vel), sizeof(float));
		sim_write.write(reinterpret_cast<const char*>(&mat.material_type), sizeof(uint32_t));
	}

	sim_write.close();

}

