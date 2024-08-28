#include <ChunkHandler.hpp>
#include <cstring>
#include <iostream>
#include <cstdint>
#include <cassert>


Material* ChunkHandler::get_material(size_t x, size_t y)
{
	if(x < 0 || x >= world_width || y < 0 || y >= world_height)
	{
		return nullptr;
	}

	std::pair<uint16_t, uint16_t> chunk_coords = std::make_pair(x/chunk_width, y/chunk_height);
	if(chunks.find(chunk_coords) == chunks.end())
	{
		return nullptr;
	}

	Chunk* chunk = chunks[chunk_coords];
	Material* material = chunk->particles[index(x, y)];

	return material;
}

ChunkHandler::Chunk* ChunkHandler::move_material(Chunk* chunk, Material* material, std::pair<uint16_t, uint16_t> old_pos, std::pair<uint16_t, uint16_t> new_pos)
{
	if(get_material(new_pos.first, new_pos.second) != nullptr)
	{
		assert(false);
		return chunk;
	}
	std::pair<uint16_t, uint16_t> new_coords = std::make_pair(new_pos.first/chunk_width, new_pos.second/chunk_height);

	Chunk* new_chunk = nullptr;
	if(chunks.find(new_coords) == chunks.end())
	{
		new_chunk = new Chunk();
		new_chunk->particles = new Material*[chunk_width * chunk_height];
		set_null(new_chunk->particles);
		bool cor = correct(new_chunk->particles);
		assert(cor);
		new_chunk->coords = new_coords;
		new_chunk->particles[index(new_pos.first, new_pos.second)] = material;
		chunk->particles[index(old_pos.first, old_pos.second)] = nullptr;
		chunk->num_materials --;
		new_chunk->num_materials ++;
		iter_chunks.push_back(new_chunk);
		chunks[new_coords] = new_chunk;
		material->position = new_pos;
		return new_chunk;
	}

	if(chunks[new_coords] == chunk)
	{
		chunk->particles[index(new_pos.first, new_pos.second)] = material;
		chunk->particles[index(old_pos.first, old_pos.second)] = nullptr;
		material->position = new_pos;
		return chunk;
	}

	if(chunks[new_coords] != chunk)
	{
		new_chunk = chunks[new_coords];
		new_chunk->particles[index(new_pos.first, new_pos.second)] = material;
		std::cout << std::to_string(index(old_pos.first, old_pos.second)) << '\n';
		chunk->particles[index(old_pos.first, old_pos.second)] = nullptr;
		chunk->num_materials --;
		new_chunk->num_materials ++;
		material->position = new_pos;
		return new_chunk;
	}
	
	return nullptr;


}

bool ChunkHandler::correct(Material** material)
{
	for(size_t i = 0; i < chunk_width * chunk_height; i ++)
	{
		if(material[i] != nullptr)
			return false;
	}
	return true;
}

void ChunkHandler::update_down(Chunk* chunk, Material* material)
{
	Material* check_material = get_material(material->position.first, material->position.second + 1);
	if(check_material == nullptr)
	{
		std::pair<uint16_t, uint16_t> new_pos = std::make_pair(material->position.first, material->position.second + 1);
		Chunk* chunk = move_material(chunk, material, material->position, new_pos);
		assert(chunk != nullptr);
	}
	else
	{
		material->velocity.second = 0;
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
		bool left_clear = get_material(material->position.first - 1, material->position.second) == nullptr;
		bool diag_left_clear = get_material(material->position.first - 1, material->position.second + 1) == nullptr;
		can_move = left_clear && diag_left_clear;
	}
	else
	{
		bool right_clear = get_material(material->position.first + 1, material->position.second) == nullptr;
		bool diag_right_clear = get_material(material->position.first + 1, material->position.second + 1) == nullptr;
		can_move = right_clear && diag_right_clear;
	}

	if(!can_move && direction == 0)
	{
		bool right_clear = get_material(material->position.first + 1, material->position.second) == nullptr;
		bool diag_right_clear = get_material(material->position.first + 1, material->position.second + 1) == nullptr;
		can_move = right_clear && diag_right_clear;
		direction = 1;
	}
	else if(!can_move && direction == 1)
	{
		bool left_clear = get_material(material->position.first - 1, material->position.second) == nullptr;
		bool diag_left_clear = get_material(material->position.first - 1, material->position.second + 1) == nullptr;
		can_move = left_clear && diag_left_clear;
		direction = 0;
	}

	if(!can_move)
	{
		return;
	}

	if(direction == 0)
	{
		std::pair<uint16_t, uint16_t> new_pos = std::make_pair(material->position.first - 1, material->position.second + 1);
		move_material(chunk, material, material->position, new_pos);
	}
	else if(direction == 1)
	{
		std::pair<uint16_t, uint16_t> new_pos = std::make_pair(material->position.first + 1, material->position.second + 1);
		move_material(chunk, material, material->position, new_pos);
	}

}

void ChunkHandler::update_chunk(Chunk* chunk)
{
	size_t chunk_length = chunk_width * chunk_height;
	Material** particles = chunk->particles;

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

void ChunkHandler::init_chunk_handler(uint16_t cW, uint16_t cH, uint16_t wW, uint16_t wH)
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

void ChunkHandler::add_materials(Material** material, size_t num_materials, PoolArena* arena)
{
	for(size_t i = 0; i < num_materials; i ++)
	{
		Material* mat = material[i];
		uint16_t x = mat->position.first;
		uint16_t y = mat->position.second;

		uint16_t chunk_posx = x/chunk_width; 
		uint16_t chunk_posy = y/chunk_height;

		std::pair<uint16_t, uint16_t> chunk_pos = std::make_pair(chunk_posx, chunk_posy);
		if(chunks.find(chunk_pos) == chunks.end())
		{
			Chunk* chunk = new Chunk();
			chunk->coords = chunk_pos;
			chunk->particles = new Material*[chunk_width * chunk_height];
			set_null(chunk->particles);
			bool cor = correct(chunk->particles);
			assert(cor);
			chunk->particles[index(x, y)] = mat;
			chunks[chunk_pos] = chunk;
			iter_chunks.push_back(chunk);
			chunk->num_materials = 0;
			chunk->num_materials ++;
		}
		else if(chunks[chunk_pos]->particles[index(x, y)] == nullptr)
		{
			chunks[chunk_pos]->particles[index(x, y)] = mat;
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

void ChunkHandler::set_null(Material** material)
{
	for(size_t i = 0; i < chunk_width * chunk_height; i ++)
	{
		material[i] = nullptr;
	}
}

void ChunkHandler::draw_chunk(Chunk* chunk, Renderer render)
{
	Material** particles = chunk->particles;
	std::pair<uint16_t, uint16_t> mat_size = std::make_pair(1, 1);
	for(size_t i = 0; i < chunk_size; i ++)
	{
		Material* mat = particles[i];
		if(particles[i] != nullptr)
		{
			render.render(mat->tex_offset, mat->position, mat_size);
		}
	}
}

size_t ChunkHandler::index(uint16_t x, uint16_t y)
{
	size_t i = ((y % chunk_height) * chunk_width) + (x % chunk_width);
	std::cout << std::to_string(i) << '\n';
	assert(i > 0 && i < chunk_width * chunk_height);
	return ((y % chunk_height) * chunk_width) + (x % chunk_width);
};
