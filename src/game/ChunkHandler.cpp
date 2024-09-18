#include <iostream>
#include <ChunkHandler.hpp>
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
	logger.log("New chunk " + print_pos(new_chunk->coords.x, new_chunk->coords.y) + " allocated.");
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


	assert(chunks.find(chunk_coords) != chunks.end());
	Chunk* chunk = chunks[chunk_coords];
	assert(index(x, y) < chunk_width * chunk_height && index(x, y) >= 0);
	Material* material = chunk->materials[index(x, y)];

	return material;
}

ChunkHandler::Chunk* ChunkHandler::move_material(Chunk* chunk, Material* material, vector2* old_pos, vector2* new_pos)
{
	if(get_material(new_pos->x, new_pos->y) != nullptr)
	{
		return chunk;
	}
	assert(material->position.x == old_pos->x && material->position.y == old_pos->y);
	vector2 new_coords {new_pos->x/chunk_width, new_pos->y/chunk_height};
	logger.log("Moving materials in chunk " + print_pos(chunk->coords.x, chunk->coords.y));
	logger.log("Moving material at position " + print_pos(old_pos->x, old_pos->y) + " to position " + print_pos(new_pos->x, new_pos->y));

	if(chunks.find(new_coords) == chunks.end())
	{
		Chunk* new_chunk = init_chunk(new_pos->x, new_pos->y);
		assert(chunk->materials[index(old_pos->x, old_pos->y)] != nullptr);
		chunk->materials[index(old_pos->x, old_pos->y)] = nullptr;
		chunk->num_materials --;
		assert(*chunk->update_list.find(material) == material);
		chunk->update_list.erase(material);
		new_chunk->materials[index(new_pos->x, new_pos->y)] = material;
		new_chunk->update_list.insert(material);
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
		assert(*chunk->update_list.find(material) == material);
		chunk->update_list.erase(material);
		chunk->num_materials --;
		new_chunk->materials[index(new_pos->x, new_pos->y)] = material;
		new_chunk->update_list.insert(material);
		new_chunk->num_materials ++;
		material->position.x = new_pos->x;
		material->position.y = new_pos->y;
		return new_chunk;
	}
	else 
	{
		assert(false);
	}
	
	return nullptr;
}

bool ChunkHandler::in_rect(Chunk* chunk, vector2* pos)
{
	return pos->x >= chunk->d_upper.x && pos->x <= chunk->d_lower.x && pos->y >= chunk->d_upper.y && pos->y <= chunk->d_lower.y;
}

void ChunkHandler::expand_rect(Chunk* chunk, vector2* old_pos, vector2* new_pos)
{
	dirty.log("Old upper dirty rect coords: " + print_pos(chunk->d_upper.x, chunk->d_upper.y) +  " Old lower dirty rect coords: " + print_pos(chunk->d_lower.x, chunk->d_lower.y));
	int upper_x, upper_y, lower_x, lower_y;

	upper_x = old_pos->x < new_pos->x? old_pos->x: new_pos->x;
	upper_y = old_pos->y < new_pos->y? old_pos->y: new_pos->y;
	lower_x = old_pos->x > new_pos->x? old_pos->x + 1: new_pos->x + 1;
	lower_y = old_pos->y > new_pos->y? old_pos->y + 1: new_pos->y + 1;

	vector2 upper{upper_x, upper_y};
	vector2 lower{lower_x, lower_y};

	Chunk* lower_chunk = get_chunk(lower.x, lower.y);
	Chunk* upper_chunk = get_chunk(upper.x, upper.y);

	if(chunk == upper_chunk)
	{
		chunk->d_upper.x = upper.x < chunk->d_upper.x || chunk->d_upper.x == -1? upper.x: chunk->d_upper.x;
		chunk->d_upper.y = upper.y < chunk->d_upper.y || chunk->d_upper.y == -1? upper.y: chunk->d_upper.y;
		chunk->d_lower.x = lower.x > chunk->d_lower.x || chunk->d_lower.x == -1? lower.x: chunk->d_lower.x;
		chunk->d_lower.y = lower.y > chunk->d_lower.y || chunk->d_lower.y == -1? lower.y: chunk->d_lower.y;
	}


	if(chunk != upper_chunk && chunk == lower_chunk)
	{
		dirty.log("Expanding to position in upper chunk" + print_pos(old_pos->x, old_pos->y) + " and " + print_pos(new_pos->x, new_pos->y));
		vector2 new_chunk_coords{upper.x/chunk_width, upper.y/chunk_height};

		if(new_chunk_coords.x == chunk->coords.x && new_chunk_coords.y < chunk->coords.y)
		{
			chunk->d_upper.y = (chunk->coords.y * chunk_height);
			chunk->d_upper.x = upper.x < chunk->d_upper.x || chunk->d_upper.x == -1? upper.x: chunk->d_upper.x;
		}
		else if(new_chunk_coords.x < chunk->coords.x && new_chunk_coords.y == chunk->coords.y)
		{
			chunk->d_upper.x = (chunk->coords.x * chunk_width);
			chunk->d_upper.y = upper.y < chunk->d_upper.y || chunk->d_upper.y == -1? upper.y: chunk->d_upper.y;
		}
		else if(new_chunk_coords.x < chunk->coords.x && new_chunk_coords.y < chunk->coords.y)
		{
			chunk->d_upper.x = chunk->coords.x * chunk_width;
			chunk->d_upper.y = chunk->coords.y * chunk_height;
		}
	}
	if(chunk != lower_chunk && chunk != upper_chunk)
	{
		vector2 lower_chunk_coords {lower.x/chunk_width, lower.y/chunk_height};
		vector2 upper_chunk_coords {upper.x/chunk_width, upper.y/chunk_height};

		if(upper_chunk_coords.x < chunk->coords.x && lower_chunk_coords.y > chunk->coords.y)
		{
			chunk->d_upper.x = chunk->coords.x * chunk_width;
			chunk->d_lower.y = (chunk->coords.y * chunk_height) + chunk_height - 1;
		}
		if(upper_chunk_coords.y > chunk->coords.y && lower_chunk_coords.x > chunk->coords.x)
		{
			chunk->d_upper.y = chunk->coords.y * chunk_height;
			chunk->d_lower.x = (chunk->coords.x * chunk_width) + chunk_width - 1;
		}
	}
	if(chunk != lower_chunk && chunk == upper_chunk)
	{

		dirty.log("Expanding to position in lower chunk " + print_pos(old_pos->x, old_pos->y) + " and " + print_pos(new_pos->x, new_pos->y));
		vector2 new_chunk_coords{lower.x/chunk_width, lower.y/chunk_height};

		if(new_chunk_coords.x == chunk->coords.x && new_chunk_coords.y > chunk->coords.y)
		{
			chunk->d_lower.y = (chunk->coords.y * chunk_height) + chunk_height - 1;
			chunk->d_lower.x = lower.x > chunk->d_lower.x || chunk->d_lower.x == -1? lower.x: chunk->d_lower.x;
		}
		else if(new_chunk_coords.x > chunk->coords.x && new_chunk_coords.y == chunk->coords.y)
		{
			chunk->d_lower.x = (chunk->coords.x * chunk_width) + chunk_width - 1;
			chunk->d_lower.y = lower.y > chunk->d_lower.y || chunk->d_lower.y == -1? lower.y: chunk->d_lower.y;
		}
		else if(new_chunk_coords.x > chunk->coords.x && new_chunk_coords.y > chunk->coords.y)
		{
			chunk->d_lower.x = (chunk->coords.x * chunk_width) + chunk_width - 1;
			chunk->d_lower.y = (chunk->coords.y * chunk_height) + chunk_height - 1;
		}

	}
	if(chunk->d_lower.x == chunk->d_upper.x && chunk->d_lower.y == chunk->d_upper.y)
	{
		chunk->d_upper.x --;
		chunk->d_upper.y --;
		logger.log("Old pos: " + print_pos(old_pos->x, old_pos->y) + " New Pos: " + print_pos(new_pos->x, new_pos->y) + "New upper dirty rect coords: " + print_pos(chunk->d_upper.x, chunk->d_upper.y) +  " New lower dirty rect coords: " + print_pos(chunk->d_lower.x, chunk->d_lower.y));
	}

	dirty.log("New upper dirty rect coords: " + print_pos(chunk->d_upper.x, chunk->d_upper.y) +  " New lower dirty rect coords: " + print_pos(chunk->d_lower.x, chunk->d_lower.y));

}

void ChunkHandler::make_dirty_rect(Chunk* chunk)
{
	logger.log("Making dirty rect for chunk " + print_pos(chunk->coords.x, chunk->coords.y));
	
	for(auto i = chunk->update_list.begin(); i != chunk->update_list.end(); i ++)
	{
		Material* mat = (*i);

		if(mat->property & Properties::STATIC)
		{
			continue;
		}

		assert(mat != nullptr);
		bool down = in_world(mat->position.x, mat->position.y + 1) && get_material(mat->position.x, mat->position.y + 1) == nullptr;
		bool side_left = in_world(mat->position.x - 1, mat->position.y) && get_material(mat->position.x - 1, mat->position.y) == nullptr;
		bool side_right = in_world(mat->position.x + 1, mat->position.y) && get_material(mat->position.x + 1, mat->position.y) == nullptr;
		bool diag_right = in_world(mat->position.x + 1, mat->position.y + 1) && get_material(mat->position.x + 1, mat->position.y + 1) == nullptr && side_right;
		bool diag_left = in_world(mat->position.x - 1, mat->position.y + 1) && get_material(mat->position.x - 1, mat->position.y + 1) == nullptr && side_left;

		if(mat->property & Properties::DOWN && down)
		{
			vector2 new_pos {mat->position.x, mat->position.y + 1};
			expand_rect(chunk, &mat->position, &new_pos);
		}
		else if(mat->property & Properties::DOWN_SIDE && diag_right && diag_left)
		{
			vector2 left {mat->position.x - 1, mat->position.y + 1};
			vector2 right {mat->position.x + 1, mat->position.y + 1};
			expand_rect(chunk, &mat->position, &left);
			expand_rect(chunk, &mat->position, &right);
			
		}
		else if(mat->property & Properties::DOWN_SIDE && diag_right)
		{
			vector2 right {mat->position.x + 1, mat->position.y + 1};
			expand_rect(chunk, &mat->position, &right);
		}
		else if(mat->property & Properties::DOWN_SIDE && diag_left)
		{
			vector2 left {mat->position.x - 1, mat->position.y + 1};
			expand_rect(chunk, &mat->position, &left);
		}
		else if(mat->property & Properties::SIDE && side_left && side_right)
		{
			vector2 left {mat->position.x - 1, mat->position.y};
			vector2 right {mat->position.x + 1, mat->position.y};
			expand_rect(chunk, &mat->position, &left);
			expand_rect(chunk, &mat->position, &right);
		}
		else if(mat->property & Properties::SIDE && side_right)
		{
			vector2 right {mat->position.x + 1, mat->position.y};
			expand_rect(chunk, &mat->position, &right);
			dirty.log("Dirty rect upper coords: " + print_pos(chunk->d_upper.x, chunk->d_upper.y) + " Dirty rect lower coords: " + print_pos(chunk->d_lower.x, chunk->d_lower.y));
		}
		else if(mat->property & Properties::SIDE && side_left)
		{
			vector2 left {mat->position.x - 1, mat->position.y};
			expand_rect(chunk, &mat->position, &left);
		}
	}
}


bool ChunkHandler::update_down(Chunk* chunk, Material* material)
{
	Material* check_material = get_material(material->position.x, material->position.y + 1);

	
	if(check_material == nullptr && in_world(material->position.x, material->position.y + 1))
	{
		vector2 new_pos {material->position.x, material->position.y + 1};
		Move new_move {material->position, new_pos};
		chunk->moves.push_back(new_move);
		return true;
	}
	return false;
}

bool ChunkHandler::update_side(Chunk* chunk, Material* material)
{
	bool left = material->velocity.x < 0;
	bool right = material->velocity.x > 0; 
	if(!left || !right)
	{
		uint8_t direction = rand() % 2;
		left = direction == 0;
		right = direction == 1;
	}

	if(left && get_material(material->position.x - 1, material->position.y) == nullptr)
	{
		vector2 new_pos {material->position.x - 1, material->position.y};
		Move move {material->position, new_pos};
		assert(move.old_pos == material->position);
		chunk->moves.push_back(move);
		return true;
	}
	if(right && get_material(material->position.x + 1, material->position.y) == nullptr)
	{
		vector2 new_pos {material->position.x + 1, material->position.y};
		Move move {material->position, new_pos};
		chunk->moves.push_back(move);
		return true;
	}

	if(left && get_material(material->position.x - 1, material->position.y) != nullptr && get_material(material->position.x + 1, material->position.y) == nullptr)
	{
		vector2 new_pos {material->position.x + 1, material->position.y};
		Move move {material->position, new_pos};
		chunk->moves.push_back(move);
		return true;
	}
	if(right && get_material(material->position.x + 1, material->position.y) != nullptr && get_material(material->position.x - 1, material->position.y) == nullptr)
	{
		vector2 new_pos {material->position.x - 1, material->position.y};
		Move move {material->position, new_pos};
		chunk->moves.push_back(move);
		return true;
	}
	return false;
}

bool ChunkHandler::update_side_down(Chunk* chunk, Material* material)
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
		Move new_move {material->position, new_pos};
		chunk->moves.push_back(new_move);
		return true;
	}
	else if(direction == 1)
	{
		vector2 new_pos {material->position.x + 1, material->position.y + 1};
		Move new_move {material->position, new_pos};
		chunk->moves.push_back(new_move);
		return true;
	}
	return false;
}

void ChunkHandler::update_chunk(Chunk* chunk)
{
	assert(chunks.find(chunk->coords) != chunks.end());
	std::vector<Material*>& particles = chunk->materials;
	
	make_dirty_rect(chunk);
	logger.log("Dirty rect upper pos: " + print_pos(chunk->d_upper.x, chunk->d_upper.y) + " Lower rect upper pos: " + print_pos(chunk->d_lower.x, chunk->d_lower.y));

	int begin = index(chunk->d_upper.x, chunk->d_upper.y), end = index(chunk->d_lower.x, chunk->d_lower.y) + 1;
	if(begin < 0 || end < 0)
	{
		return;
	}
	logger.log("Materials list begin and end position " + print_pos(begin, end) + " for chunk " + print_pos(chunk->coords.x, chunk->coords.y));
	logger.log("Updating chunk " + print_pos(chunk->coords.x, chunk->coords.y));
	if(begin >= 4800)
	{
		std::cout << "Bad\n";
	}
	for(size_t i = begin; i < end; i ++)
	{
		Material* mat = particles[i];
		if(mat == nullptr)
		{
			continue;
		}

		bool moved = false;
		if(mat->property & Properties::DOWN && !moved)
		{
			moved = update_down(chunk, mat);
		}
		if(mat->property & Properties::DOWN_SIDE && !moved)
		{
			moved = update_side_down(chunk, mat);
		}
		if(mat->property & Properties::SIDE && !moved)
		{
			moved = update_side(chunk, mat);
		}

	}
	if(chunk->num_materials == 0)
	{
		logger.log("Freeing chunk " + print_pos(chunk->coords.x, chunk->coords.y));
	}
}

ChunkHandler::Chunk* ChunkHandler::get_chunk(int x, int y)
{
	vector2 chunk_coords {x/chunk_width, y/chunk_height};
	if(chunks.find(chunk_coords) == chunks.end())
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
	dirty.init_logger("dirty.txt");
	mat_logger.init_logger("materials.txt");
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
			chunk->update_list.insert(mat);
			chunk->materials[index(x, y)] = mat;
			chunk->num_materials ++;
		}
		else if(chunks[chunk_pos]->materials[index(x, y)] == nullptr)
		{
			chunks[chunk_pos]->update_list.insert(mat);
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
	if(chunk->num_materials == 0)
	{
		return;
	}

	std::vector<Move> iter_moves = chunk->moves;

	for(size_t i = 0; i < iter_moves.size(); i ++)
	{
		int rand_index = rand() % iter_moves.size();
		
		Move move = iter_moves[rand_index];
		Material* material = get_material(move.old_pos.x, move.old_pos.y);
		assert(material != nullptr);
		assert(material->position.x == move.old_pos.x && material->position.y == move.old_pos.y);
		move_material(chunk, material, &move.old_pos, &move.new_pos);
		iter_moves.erase(iter_moves.begin() + rand_index);
	}
	assert(chunk->update_list.size() == chunk->num_materials);
	chunk->moves.clear();
}

void ChunkHandler::draw_chunk(Chunk* chunk, Renderer render)
{
	const std::vector<Material*>& particles = chunk->materials;
	vector2 end_coords;
	end_coords.x = (chunk->coords.x * chunk_width) + chunk_width;
	end_coords.y = (chunk->coords.y * chunk_height) + chunk_height;
	vector2 start_coords {chunk->coords.x * chunk_width, chunk->coords.y * chunk_height};
	render.draw_rect(start_coords, end_coords, 4.0f/5.0f);
	render.draw_rect(chunk->d_upper, chunk->d_lower, 3.0f/5.0f);
	vector2 mat_size {1, 1};
	for(size_t i = 0; i < chunk_size; i ++)
	{
		Material* mat = particles[i];
		if(particles[i] != nullptr)
		{
			render.render(mat->tex_offset, &mat->position, &mat_size);
		}
	}
	chunk->d_lower.x = -1;
	chunk->d_lower.y = -1;
	chunk->d_upper.x = -1;
	chunk->d_upper.y = -1;
}

size_t ChunkHandler::index(int x, int y)
{
	size_t i = ((y % chunk_height) * chunk_width) + (x % chunk_width);
	//assert(i >= 0 && i < chunk_width * chunk_height);
	return ((y % chunk_height) * chunk_width) + (x % chunk_width);
};
