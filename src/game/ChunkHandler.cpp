#include <ChunkHandler.hpp>
#include <iostream>
#include <cstring>
#include <cstdint>
#include <cassert>



std::string ChunkHandler::print_pos(int x, int y)
{
	return "(" + std::to_string(x) + ", " + std::to_string(y) + ")"; 
}

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
	return new_chunk;
}

bool ChunkHandler::in_world(int x, int y)
{
	return x >= 0 && x < world_width && y >= 0 && y < world_height;
}

Material* ChunkHandler::get_material(int x, int y)
{

	vector2 chunk_coords {x/chunk_width, y/chunk_height};

	if(chunks.find(chunk_coords) == chunks.end())
	{
		return nullptr;
	}

	Chunk* chunk = chunks[chunk_coords];
	Material* material = chunk->materials[index(x, y)];

	return material;
}

ChunkHandler::Chunk* ChunkHandler::move_material(Chunk* chunk, Material* material, size_t erase_index, vector2* old_pos, vector2* new_pos)
{
	if(get_material(new_pos->x, new_pos->y) != nullptr)
	{
		return chunk;
	}
	vector2 new_coords {new_pos->x/chunk_width, new_pos->y/chunk_height};

	if(chunks.find(new_coords) == chunks.end())
	{
		Chunk* new_chunk = init_chunk(new_pos->x, new_pos->y);
		assert(chunk->materials[index(old_pos->x, old_pos->y)] != nullptr);
		chunk->materials[index(old_pos->x, old_pos->y)] = nullptr;
		chunk->num_materials --;
		assert(chunk->update_list[erase_index] == material);
		chunk->update_list[erase_index] = nullptr;
		new_chunk->materials[index(new_pos->x, new_pos->y)] = material;
		new_chunk->update_list.push_back(material);
		new_chunk->num_materials ++;
		material->position.x = new_pos->x;
		material->position.y = new_pos->y;
		return new_chunk;
	}
	else if(chunks[new_coords] == chunk)
	{
		assert(chunk->materials[index(old_pos->x, old_pos->y)] != nullptr);
		assert(chunk->materials[index(new_pos->x, new_pos->y)] == nullptr);
		chunk->materials[index(new_pos->x, new_pos->y)] = material;
		chunk->materials[index(old_pos->x, old_pos->y)] = nullptr;
		material->position.x = new_pos->x;
		material->position.y = new_pos->y;
		return chunk;
	}
	else if(chunks[new_coords] != chunk)
	{
		Chunk* new_chunk = chunks[new_coords];
		assert(chunk->materials[index(old_pos->x, old_pos->y)] != nullptr);
		assert(new_chunk->materials[index(new_pos->x, new_pos->y)] == nullptr);
		chunk->materials[index(old_pos->x, old_pos->y)] = nullptr;
		assert(chunk->update_list[erase_index] == material);
		chunk->update_list[erase_index] = nullptr;
		chunk->num_materials --;
		new_chunk->materials[index(new_pos->x, new_pos->y)] = material;
		new_chunk->update_list.push_back(material);
		new_chunk->num_materials ++;
		material->position.x = new_pos->x;
		material->position.y = new_pos->y;
		return new_chunk;
	}
	
	return nullptr;
}



bool ChunkHandler::update_down(Chunk* chunk, Material* material, size_t index)
{
	Material* check_material = get_material(material->position.x, material->position.y + 1);
	logger.log("Moving Material " + print_pos(material->position.x, material->position.y) + " to position " + print_pos(material->position.x, material->position.y) + " in " + print_pos(chunk->coords.x, chunk->coords.y));

	
	if(check_material == nullptr && in_world(material->position.x, material->position.y + 1))
	{
		vector2 new_pos {material->position.x, material->position.y + 1};
		Move new_move {material->position, new_pos, index};
		chunk->moves.push_back(new_move);
		return true;
	}
	material->velocity.y = 0;
	return false;
}

bool ChunkHandler::update_side(Chunk* chunk, Material* material, size_t index)
{
	bool left = material->velocity.x < 0;
	bool right = material->velocity.x > 0; 
	if(!left || !right && material->velocity.y == 0)
	{
		uint8_t direction = rand() % 2;
		left = direction == 0;
		right = direction == 1;
	}

	if(left && get_material(material->position.x - 1, material->position.y) == nullptr)
	{
		vector2 new_pos {material->position.x - 1, material->position.y};
		Move move {material->position, new_pos, index};
		chunk->moves.push_back(move);
		material->velocity.x = -1;
		return true;
	}
	if(right && get_material(material->position.x + 1, material->position.y) == nullptr)
	{
		vector2 new_pos {material->position.x + 1, material->position.y};
		Move move {material->position, new_pos, index};
		chunk->moves.push_back(move);
		material->velocity.x = 1;
		return true;
	}

	if(left && get_material(material->position.x - 1, material->position.y) != nullptr && get_material(material->position.x + 1, material->position.y) == nullptr)
	{
		vector2 new_pos {material->position.x + 1, material->position.y};
		Move move {material->position, new_pos, index};
		chunk->moves.push_back(move);
		material->velocity.x = 1;
		return true;
	}
	if(right && get_material(material->position.x + 1, material->position.y) != nullptr && get_material(material->position.x - 1, material->position.y) == nullptr)
	{
		vector2 new_pos {material->position.x - 1, material->position.y};
		Move move {material->position, new_pos, index};
		chunk->moves.push_back(move);
		material->velocity.x = -1;
		return true;
	}
	return false;
}

bool ChunkHandler::update_side_down(Chunk* chunk, Material* material, size_t index)
{
	uint8_t direction = rand() % 2;
	bool can_move = false;

	if(direction == 0)
	{
		bool left_clear = get_material(material->position.x - 1, material->position.y) == nullptr;
		bool diag_left_clear = get_material(material->position.x - 1, material->position.y + 1) == nullptr;
		can_move = left_clear && diag_left_clear && in_world(material->position.x - 1, material->position.y + 1);
	}
	else
	{
		bool right_clear = get_material(material->position.x + 1, material->position.y) == nullptr;
		bool diag_right_clear = get_material(material->position.x + 1, material->position.y + 1) == nullptr;
		can_move = right_clear && diag_right_clear && in_world(material->position.x + 1, material->position.y + 1);
	}

	if(!can_move && direction == 0)
	{
		bool right_clear = get_material(material->position.x + 1, material->position.y) == nullptr;
		bool diag_right_clear = get_material(material->position.x + 1, material->position.y + 1) == nullptr;
		can_move = right_clear && diag_right_clear && in_world(material->position.x + 1, material->position.y + 1);
		direction = 1;
	}
	else if(!can_move && direction == 1)
	{
		bool left_clear = get_material(material->position.x - 1, material->position.y) == nullptr;
		bool diag_left_clear = get_material(material->position.x - 1, material->position.y + 1) == nullptr;
		can_move = left_clear && diag_left_clear && in_world(material->position.x - 1, material->position.y + 1);
		direction = 0;
	}

	if(!can_move)
	{
		return false;
	}

	if(direction == 0)
	{
		vector2 new_pos {material->position.x - 1, material->position.y + 1};
		Move new_move {material->position, new_pos, index};
		chunk->moves.push_back(new_move);
		return true;
	}
	else if(direction == 1)
	{
		vector2 new_pos {material->position.x + 1, material->position.y + 1};
		Move new_move {material->position, new_pos, index};
		chunk->moves.push_back(new_move);
		return true;
	}
	return false;
}

void ChunkHandler::update_chunk(Chunk* chunk)
{
	assert(chunks.find(chunk->coords) != chunks.end());
	std::vector<Material*>& particles = chunk->update_list;
	
	if(chunk->static_materials == chunk->num_materials)
	{
		return;
	}

	for(size_t i = 0; i < particles.size(); i ++)
	{


		Material* mat = particles[i];
		if(mat == nullptr)
		{
			continue;
		}

		bool moved = false;
		if(mat->property & Properties::DOWN && !moved)
		{
			moved = update_down(chunk, mat, i);
		}
		if(mat->property & Properties::DOWN_SIDE && !moved)
		{
			moved = update_side_down(chunk, mat, i);
		}
		if(mat->property & Properties::SIDE && !moved)
		{
			moved = update_side(chunk, mat, i);
		}
		mat->updated = moved;

	}
}

ChunkHandler::Chunk* ChunkHandler::get_chunk(int x, int y)
{
	vector2 chunk_coords {x/chunk_width, y/chunk_height};
	if(chunks.find(chunk_coords) != chunks.end())
	{
		return nullptr;
	}
	return chunks[chunk_coords];
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
	logger.init_logger("chunk_log.txt");
}

void ChunkHandler::add_materials(const std::vector<Material*>& material, PoolArena* arena)
{
	for(size_t i = 0; i < material.size(); i ++)
	{
		Material* mat = material[i];
		int x = mat->position.x;
		int y = mat->position.y;

		vector2 chunk_pos {x/chunk_width, y/chunk_height};

		if(chunks.find(chunk_pos) == chunks.end())
		{
			Chunk* chunk = init_chunk(x, y);
			assert(chunk != nullptr);
			chunk->update_list.push_back(mat);
			chunk->materials[index(x, y)] = mat;
			chunk->num_materials ++;
		}
		else if(chunks[chunk_pos]->materials[index(x, y)] == nullptr)
		{
			chunks[chunk_pos]->update_list.push_back(mat);
			chunks[chunk_pos]->materials[index(x, y)] = mat;
			chunks[chunk_pos]->num_materials ++;
		}
		else
		{
			int success = deallocate(arena, mat);
			if(success == -1)
			{
				assert(false);
			}
		}
	}
}


void ChunkHandler::commit_changes(Chunk* chunk)
{
	std::vector<Move> iter_moves = chunk->moves;
	std::vector<Material*>& materials = chunk->update_list;
	for(size_t i = 0; i < iter_moves.size(); i ++)
	{
		int rand_index = rand() % iter_moves.size();
		
		Move move = iter_moves[rand_index];
		Material* material = get_material(move.old_pos.x, move.old_pos.y);
		assert(material != nullptr);
		move_material(chunk, material, move.index, &move.old_pos, &move.new_pos);
		iter_moves.erase(iter_moves.begin() + rand_index);
	}

	for(size_t i = 0; i < materials.size(); i ++)
	{
		if(materials[i] == nullptr)
		{
			materials.erase(materials.begin() + i);
		}
	}
	chunk->moves.clear();
}

void ChunkHandler::draw_chunk(Chunk* chunk, Renderer render)
{
	const std::vector<Material*>& particles = chunk->materials;
	vector2 mat_size {1, 1};
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
