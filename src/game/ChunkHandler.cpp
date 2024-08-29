#include <ChunkHandler.hpp>
#include <cstring>
#include <iostream>
#include <cstdint>
#include <cassert>

ChunkHandler::Chunk* ChunkHandler::init_chunk(int mat_x, int mat_y)
{
	Chunk* new_chunk = new Chunk();
	assert(new_chunk != nullptr);
	new_chunk->coords.x = mat_x/chunk_width;
	new_chunk->coords.y = mat_y/chunk_height;
	new_chunk->materials.resize(chunk_width * chunk_height);
	assert(new_chunk->materials.size() == chunk_width * chunk_height);
	new_chunk->num_materials = 0;
	iter_chunks.push_back(new_chunk);
	chunks[new_chunk->coords] = new_chunk;
	std::cout << "Chunk (" << std::to_string(new_chunk->coords.x) << ", " << std::to_string(new_chunk->coords.y) << ") created.\n";
	return new_chunk;
}

Material* ChunkHandler::get_material(size_t x, size_t y)
{
	if(x < 0 || x >= world_width || y < 0 || y >= world_height)
	{
		return nullptr;
	}

	vector2 chunk_coords;
	chunk_coords.x = x/chunk_width;
	chunk_coords.y = y/chunk_height;

	if(chunks.find(chunk_coords) == chunks.end())
	{
		return nullptr;
	}

	Chunk* chunk = chunks[chunk_coords];
	Material* material = chunk->materials[index(x, y)];

	return material;
}

ChunkHandler::Chunk* ChunkHandler::move_material(Chunk* chunk, Material* material, vector2* old_pos, vector2* new_pos)
{
	if(get_material(new_pos->x, new_pos->y) != nullptr)
	{
		assert(false);
		return chunk;
	}
	vector2 new_coords;
	new_coords.x = new_pos->x/chunk_width;
	new_coords.y = new_pos->y/chunk_height;


	if(chunks.find(new_coords) == chunks.end())
	{
		Chunk* new_chunk = init_chunk(new_pos->x, new_pos->y);
		new_chunk->materials[index(new_pos->x, new_pos->y)] = material;
		chunk->materials[index(old_pos->x, old_pos->y)] = nullptr;
		chunk->num_materials --;
		new_chunk->num_materials ++;
		material->position.x = new_pos->x;
		material->position.y = new_pos->y;
		return new_chunk;
	}
	else if(chunks[new_coords] == chunk)
	{
		chunk->materials[index(new_pos->x, new_pos->y)] = material;
		chunk->materials[index(old_pos->x, old_pos->y)] = nullptr;
		material->position.x = new_pos->x;
		material->position.y = new_pos->y;
		return chunk;
	}
	else if(chunks[new_coords] != chunk)
	{
		Chunk* new_chunk = chunks[new_coords];
		new_chunk->materials[index(new_pos->x, new_pos->y)] = material;
		vector2 fail;
		fail.x = chunk->coords.x;
		fail.y = chunk->coords.y;
		//assert(chunks.find(chunk->coords) != chunks.end());
		std::cout << "Chunk coords not found: (" << std::to_string(fail.x) << ", " << std::to_string(fail.y) << ").\n";
		chunk->materials[index(old_pos->x, old_pos->y)] = nullptr;
		chunk->num_materials --;
		new_chunk->num_materials ++;
		material->position.x = new_pos->x;
		material->position.y = new_pos->y;
		return new_chunk;
	}
	
	return nullptr;
}



void ChunkHandler::update_down(Chunk* chunk, Material* material)
{
	Material* check_material = get_material(material->position.x, material->position.y + 1);
	if(check_material == nullptr)
	{
		vector2 new_pos;
		new_pos.x = material->position.x;
		new_pos.y = material->position.y + 1;
		Chunk* chunk = move_material(chunk, material, &material->position, &new_pos);
		assert(chunk != nullptr);
	}
	else
	{
		material->velocity.y = 0;
	}
}

void ChunkHandler::update_side(Chunk* chunk, Material* material)
{
}

void ChunkHandler::update_side_down(Chunk* chunk, Material* material)
{
	uint8_t direction = rand() % 2;
	bool can_move = false;

	if(direction == 0)
	{
		bool left_clear = get_material(material->position.x - 1, material->position.y) == nullptr;
		bool diag_left_clear = get_material(material->position.x - 1, material->position.y + 1) == nullptr;
		can_move = left_clear && diag_left_clear;
	}
	else
	{
		bool right_clear = get_material(material->position.x + 1, material->position.y) == nullptr;
		bool diag_right_clear = get_material(material->position.x + 1, material->position.y + 1) == nullptr;
		can_move = right_clear && diag_right_clear;
	}

	if(!can_move && direction == 0)
	{
		bool right_clear = get_material(material->position.x + 1, material->position.y) == nullptr;
		bool diag_right_clear = get_material(material->position.x + 1, material->position.y + 1) == nullptr;
		can_move = right_clear && diag_right_clear;
		direction = 1;
	}
	else if(!can_move && direction == 1)
	{
		bool left_clear = get_material(material->position.x - 1, material->position.y) == nullptr;
		bool diag_left_clear = get_material(material->position.x - 1, material->position.y + 1) == nullptr;
		can_move = left_clear && diag_left_clear;
		direction = 0;
	}

	if(!can_move)
	{
		return;
	}

	if(direction == 0)
	{
		vector2 new_pos;
		new_pos.x = material->position.x - 1;
		new_pos.y = material->position.y + 1;
		move_material(chunk, material, &material->position, &new_pos);
	}
	else if(direction == 1)
	{
		vector2 new_pos;
		new_pos.x = material->position.x + 1;
		new_pos.y = material->position.y + 1;
		move_material(chunk, material, &material->position, &new_pos);
	}

}

void ChunkHandler::update_chunk(Chunk* chunk)
{
	std::cout << "Updating Chunk (" << std::to_string(chunk->coords.x) << ", " << std::to_string(chunk->coords.y) << ") created.\n";
	assert(chunks.find(chunk->coords) != chunks.end());
	size_t chunk_length = chunk_width * chunk_height;
	const std::vector<Material*>& particles = chunk->materials;

	for(size_t i = 0; i < chunk_length; i ++)
	{
		Material* mat = particles[i];
		if(mat == nullptr)
		{
			continue;
		}

		if(mat->property & Properties::DOWN)
		{
			update_down(chunk, mat);
		}
		if(mat->property & Properties::DOWN_SIDE)
		{
			update_side_down(chunk, mat);
		}
		if(mat->property & Properties::SIDE)
		{
			update_side(chunk, mat);
		}
	}
}

void ChunkHandler::init_chunk_handler(int cW, int cH, int wW, int wH)
{
	if(wW % cW != 0 || wH % cH != 0)
	{
		assert(false);
		return;
	}

	chunk_width = cW;
	chunk_height = cH;

	world_width = wW;
	world_height = wH;

	x_chunks = wW/cW;
	y_chunks = wH/cH;
	chunk_size = cW * cH;
}

void ChunkHandler::add_materials(const std::vector<Material*>& material, PoolArena* arena)
{
	for(size_t i = 0; i < material.size(); i ++)
	{
		Material* mat = material[i];
		int x = mat->position.x;
		int y = mat->position.y;

		vector2 chunk_pos;
		chunk_pos.x = x/chunk_width;
		chunk_pos.y = y/chunk_height;

		if(chunks.find(chunk_pos) == chunks.end())
		{
			Chunk* chunk = init_chunk(x, y);
			assert(chunk != nullptr);
			chunk->materials[index(x, y)] = mat;
			chunk->num_materials ++;
		}
		else if(chunks[chunk_pos]->materials[index(x, y)] == nullptr)
		{
			chunks[chunk_pos]->materials[index(x, y)] = mat;
			chunks[chunk_pos]->num_materials ++;
		}
		else
		{
			int success = deallocate(arena, mat);
			if(success == -1)
			{
				std::cout << "Deallocation failed" << '\n';
				assert(false);
			}
		}
	}
}

void ChunkHandler::draw_chunk(Chunk* chunk, Renderer render)
{
	const std::vector<Material*>& particles = chunk->materials;
	vector2 mat_size;
	mat_size.x = 1;
	mat_size.y = 1;
	for(size_t i = 0; i < chunk_size; i ++)
	{
		Material* mat = particles[i];
		if(particles[i] != nullptr)
		{
			render.render(mat->tex_offset, &mat->position, &mat_size);
		}
	}
}

size_t ChunkHandler::index(int x, int y)
{
	size_t i = ((y % chunk_height) * chunk_width) + (x % chunk_width);
	assert(i >= 0 && i < chunk_width * chunk_height);
	return ((y % chunk_height) * chunk_width) + (x % chunk_width);
};
