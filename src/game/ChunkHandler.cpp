#include "Material.hpp"
#include "PoolArena.hpp"
#include <ChunkHandler.hpp>
#include <algorithm>
#include <cassert>
#include <climits>
#include <cstring>
#include <iostream>

#define TEX_COORD(MATERIAL) tex_coords.##MATERIAL

void ChunkHandler::init_chunk_handler(unsigned int x_chunks, unsigned int y_chunks, int wW, int wH, PoolArena *arena) {
  element_updater.init(this, wW, wH);
  material_arena = arena;

  chunk_width = (wW % x_chunks == 0) ? wW / x_chunks : (wW / x_chunks) + 1;
  chunk_height = (wH % y_chunks == 0) ? wH / y_chunks : (wH / y_chunks) + 1;

  world_width = wW;
  world_height = wH;

  this->x_chunks = x_chunks;
  this->y_chunks = y_chunks;
  chunk_size = chunk_width * chunk_height;


  init_material_props();

  shuffler = std::mt19937(rng());

  if (rng.entropy() == 0) {
    std::cout << "No hardward support for true randomness.\n";
  } else {
    std::cout << "Hardware randomness with entropy at value " << rng.entropy()
      << '\n';
  }
}

void ChunkHandler::debug_asleep_materials(){
  for(Chunk* chunk: iter_chunks)
  {
    for(Material* mat: chunk->asleep_list)
    {
      if(mat->position.y < 200)
      {
        debug_mat(mat);
        std::cout << '\n';
      }
    }
  }
}

bool ChunkHandler::is_active(Material* material)
{
  Chunk* chunk = get_chunk(material->position.x, material->position.y);
  if(chunk == nullptr) return false;
  return chunk->active_list.find(material) != chunk->active_list.end();
}

bool ChunkHandler::is_asleep(Material* material)
{
  Chunk* chunk = get_chunk(material->position.x, material->position.y);
  if(chunk == nullptr) return false;
  return chunk->asleep_list.find(material) != chunk->asleep_list.end();
}

void ChunkHandler::set_active(Material* material) {
  Chunk* enclosing_chunk = get_chunk(material->position.x, material->position.y);
  material->velocity.x = 0;
  material->velocity.y = 1.0f;
  enclosing_chunk->asleep_list.erase(material);
  enclosing_chunk->active_list.insert(material);
}

void ChunkHandler::set_asleep(Material* material) {
  Chunk* enclosing_chunk = get_chunk(material->position.x, material->position.y);
  enclosing_chunk->active_list.erase(material);
  enclosing_chunk->asleep_list.insert(material);
}

void ChunkHandler::remove_from_anim_list(Material *material) {
  animation_list.erase(material);
}

void ChunkHandler::debug_mat(Material *mat) {
  std::cout << "Postion: " << print_pos(mat->position.x, mat->position.y)
            << "; ";
  std::cout << "Velocity: "
            << element_updater.print_vel(mat->velocity.x, mat->velocity.y)
            << "; ";
}

bool ChunkHandler::in_anim_list(Material *material) {
  return animation_list.find(material) != animation_list.end();
}

void ChunkHandler::init_material_props() {
  material_props.resize(11);
  material_prop_arena = init_pool(11, sizeof(MaterialProps));

  material_props[static_cast<size_t>(MatType::SAND)] = (MaterialProps *)(allocate(material_prop_arena));
  material_props[static_cast<size_t>(MatType::WATER)] = (MaterialProps *)(allocate(material_prop_arena));
  material_props[static_cast<size_t>(MatType::SMOKE)] = (MaterialProps *)(allocate(material_prop_arena));
  material_props[static_cast<size_t>(MatType::FIRE)] = (MaterialProps *)(allocate(material_prop_arena));
  material_props[static_cast<size_t>(MatType::WOOD)] = (MaterialProps *)(allocate(material_prop_arena));
  material_props[static_cast<size_t>(MatType::STONE)] = (MaterialProps *)(allocate(material_prop_arena));
  material_props[static_cast<size_t>(MatType::ACID)] = (MaterialProps *)(allocate(material_prop_arena));
  material_props[static_cast<size_t>(MatType::NITRO)] = (MaterialProps *)(allocate(material_prop_arena));
  material_props[static_cast<size_t>(MatType::OIL)] = (MaterialProps *)(allocate(material_prop_arena));
  material_props[static_cast<size_t>(MatType::FLAMMABLE_GAS)] =
      (MaterialProps *)(allocate(material_prop_arena));

  *material_props[static_cast<size_t>(MatType::SAND)] = {
    MatterState::DYNAMIC_SOLID,
    static_cast<ReactionProperties>(ACID_DESTROY),
    200,
    tex_coords.SAND
  };

  *material_props[static_cast<size_t>(MatType::STONE)] = {
    MatterState::STATIC_SOLID,
    static_cast<ReactionProperties>(ACID_DESTROY),
    400,
    tex_coords.STONE
  };

  //TODO: Add the rest of the materials lookup values later
}

void ChunkHandler::set_material_properties(Material *material, MatType type,
    vector2 *pos) {
  assert(in_world(pos->x, pos->y));
  if (!material)
    return;

  material->state = ReactionState::NORMAL;
  material->mat_id = type;
  material->position.x = pos->x;
  material->position.y = pos->y;
  material->velocity.x = 0;
  material->velocity.y = 1;
  material->health = material_props[static_cast<size_t>(type)]->health;
  material->tex_offset = material_props[static_cast<size_t>(type)]->tex_offset;
}

void ChunkHandler::destroy_material(Material *material) {
  if (material == nullptr) {
    return;
  }
  vector2 pos{material->position.x, material->position.y};
  Chunk *chunk = get_chunk(pos.x, pos.y);
  assert(chunk != nullptr);
  assert(chunk->grid[index(pos.x, pos.y)] != nullptr);
  chunk->grid[index(pos.x, pos.y)] = nullptr;
  chunk->num_materials--;
  int free_failure = deallocate(material_arena, material);
  assert(free_failure != -1);
}

std::string ChunkHandler::print_pos(int x, int y) {
  return "(" + std::to_string(x) + ", " + std::to_string(y) + ")";
}

std::string ChunkHandler::print_pos(vector2 &pos) {
  return "(" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ")";
}

std::string ChunkHandler::print_pos(fvector2 &pos) {
  return "(" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ")";
}

Chunk *ChunkHandler::init_chunk(int mat_x, int mat_y) {
  Chunk *new_chunk = new Chunk();

  assert(new_chunk != nullptr);
  new_chunk->lower_rect_coords.x = INT_MIN,
    new_chunk->lower_rect_coords.y = INT_MIN;
  new_chunk->upper_rect_coords.x = INT_MAX,
    new_chunk->upper_rect_coords.y = INT_MAX;
  new_chunk->coords.x = mat_x / chunk_width;
  new_chunk->coords.y = mat_y / chunk_height;
  new_chunk->grid = new Material *[chunk_width * chunk_height];

  for (size_t i = 0; i < chunk_size; i++) {
    new_chunk->grid[i] = nullptr;
  }

  new_chunk->num_materials = 0;
  add_materials_list.push_back(new_chunk);
  chunks[new_chunk->coords] = new_chunk;
  return new_chunk;
}

bool ChunkHandler::can_react(Material *m1, Material *m2) {
  switch (m1->mat_id) {
    case MatType::SAND:
      return material_props[static_cast<size_t>(m1->mat_id)]->reaction & DISPLACIBLE;
      break;
    case MatType::ACID:
      return material_props[static_cast<size_t>(m2->mat_id)]->reaction & ACID_DESTROY;
      break;
  }
  return false;
}


void ChunkHandler::modify_rect(Chunk *chunk, vector2 *new_rect_upper, vector2 *new_rect_lower) {
  vector2 *old_rect_upper = &chunk->upper_rect_coords;
  vector2 *old_rect_lower = &chunk->lower_rect_coords;

  vector2 upper_chunk{new_rect_upper->x / chunk_width,
    new_rect_upper->y / chunk_height};
  vector2 lower_chunk{new_rect_lower->x / chunk_width,
    new_rect_lower->y / chunk_height};

  if (upper_chunk.x < chunk->coords.x && upper_chunk.y == chunk->coords.y) {
    new_rect_upper->x = chunk->coords.x * chunk_width;
  } else if (upper_chunk.x == chunk->coords.x &&
      upper_chunk.y < chunk->coords.y) {
    new_rect_upper->y = chunk->coords.y * chunk_height;
  } else if (upper_chunk.x < chunk->coords.x &&
      upper_chunk.y < chunk->coords.y) {
    new_rect_upper->x = chunk->coords.x * chunk_width;
    new_rect_upper->y = chunk->coords.y * chunk_height;
  }

  if (lower_chunk.x > chunk->coords.x && lower_chunk.y == chunk->coords.y) {
    new_rect_lower->x = chunk->coords.x * chunk_width + chunk_width - 1;
  } else if (lower_chunk.x == chunk->coords.x &&
      lower_chunk.y > chunk->coords.y) {
    new_rect_lower->y = chunk->coords.y * chunk_height + chunk_height - 1;
  } else if (lower_chunk.x > chunk->coords.x &&
      lower_chunk.y > chunk->coords.y) {
    new_rect_lower->x = chunk->coords.x * chunk_width + chunk_width - 1;
    new_rect_lower->y = chunk->coords.y * chunk_height + chunk_height - 1;
  }

  if (chunk->upper_rect_coords.x == INT_MAX) {
    chunk->upper_rect_coords.x = new_rect_upper->x;
    chunk->upper_rect_coords.y = new_rect_upper->y;
    chunk->lower_rect_coords.x = new_rect_lower->x;
    chunk->lower_rect_coords.y = new_rect_lower->y;
    return;
  }

  if (new_rect_upper->x > chunk->upper_rect_coords.x &&
      new_rect_lower->x < chunk->lower_rect_coords.x &&
      new_rect_upper->y > chunk->upper_rect_coords.y &&
      new_rect_lower->y < chunk->lower_rect_coords.y) {
    chunk->upper_rect_coords.x = new_rect_upper->x;
    chunk->upper_rect_coords.y = new_rect_upper->y;
    chunk->lower_rect_coords.x = new_rect_lower->x;
    chunk->lower_rect_coords.y = new_rect_lower->y;
    return;
  }

  if (!(new_rect_upper->x > chunk->lower_rect_coords.x ||
        new_rect_upper->y > chunk->lower_rect_coords.y ||
        new_rect_lower->x < chunk->upper_rect_coords.x ||
        new_rect_lower->y < chunk->upper_rect_coords.y)) {
    chunk->upper_rect_coords.x = chunk->upper_rect_coords.x > new_rect_upper->x
      ? new_rect_upper->x
      : chunk->upper_rect_coords.x;
    chunk->upper_rect_coords.y = chunk->upper_rect_coords.y > new_rect_upper->y
      ? new_rect_upper->y
      : chunk->upper_rect_coords.y;
    chunk->lower_rect_coords.x = chunk->lower_rect_coords.x < new_rect_lower->x
      ? new_rect_lower->x
      : chunk->lower_rect_coords.x;
    chunk->lower_rect_coords.y = chunk->lower_rect_coords.y < new_rect_lower->y
      ? new_rect_lower->y
      : chunk->lower_rect_coords.y;
  } else {
    chunk->upper_rect_coords.x = new_rect_upper->x;
    chunk->upper_rect_coords.y = new_rect_upper->y;
    chunk->lower_rect_coords.x = new_rect_lower->x;
    chunk->lower_rect_coords.y = new_rect_lower->y;
  }

  // assert(chunk->upper_rect_coords.x < chunk->lower_rect_coords.x &&
  // chunk->upper_rect_coords.y < chunk->lower_rect_coords.y);
  // assert(chunk->upper_rect_coords.x > 0 && chunk->upper_rect_coords.y > 0);
}

void ChunkHandler::make_dirty_rect(Chunk *chunk) {

  if (chunk->active_list.size() == 0) {
    chunk->upper_rect_coords.x = -1;
    chunk->upper_rect_coords.y = -1;
    chunk->lower_rect_coords.x = -1;
    chunk->lower_rect_coords.x = -1;
    return;
  }

  int min_x = INT_MAX;
  int min_y = INT_MAX;
  int max_x = INT_MIN;
  int max_y = INT_MIN;

  const std::set<Material *> &mat_list = chunk->active_list;

  for (auto i = mat_list.begin(); i != mat_list.end(); i++) {
    Material* mat = *i;

    int mat_x = mat->position.x;
    int mat_y = mat->position.y;

    int vel_x = static_cast<int>(mat->velocity.x + 0.5f);
    int vel_y = static_cast<int>(mat->velocity.y + 0.5f);

    if (material_props[static_cast<size_t>(mat->mat_id)]->mat_state != MatterState::STATIC_SOLID) {
      min_x = std::min(mat_x, min_x);
      min_y = std::min(mat_y, min_y);

      max_x = std::max(mat_x, max_x);
      max_y = std::max(mat_y, max_y);

      min_x = std::min(mat_x + vel_x, min_x);
      min_y = std::min(mat_y + vel_y, min_y);

      max_x = std::max(mat_x + vel_x, max_x);
      max_y = std::max(mat_y + vel_y, max_y);
    }

  }

  vector2 upper_rect{min_x, min_y};
  vector2 lower_rect{max_x, max_y};
  modify_rect(chunk, &upper_rect, &lower_rect);
}

void ChunkHandler::update_chunk(Chunk *chunk, const float dT) {

  if (chunk->active_list.size() == 0) {
    return;
  }

  make_dirty_rect(chunk);

  for (auto it =  chunk->active_list.begin(); it != chunk->active_list.end();) {
    Material* mat = *it;
    ++ it;

    switch (mat->mat_id)
    {
      case MatType::SAND:
        element_updater.update_sand(dT, mat);
        break;
    }
  }

}

void ChunkHandler::add_materials(const std::vector<Material *> &material) {
  for (size_t i = 0; i < material.size(); i++) {
    Material *mat = material[i];
    int x = mat->position.x;
    int y = mat->position.y;

    vector2 chunk_pos{x / chunk_width, y / chunk_height};

    if (chunks.find(chunk_pos) == chunks.end()) {
      Chunk *chunk = init_chunk(x, y);
      assert(chunk != nullptr);
      chunk->active_list.insert(mat);
      chunk->grid[index(x, y)] = mat;
      chunk->num_materials++;
    } else if (!get_material(x, y)) {
      chunks[chunk_pos]->active_list.insert(mat);
      assert(*(chunks[chunk_pos]->active_list.find(mat)) == mat);
      chunks[chunk_pos]->grid[index(x, y)] = mat;
      chunks[chunk_pos]->num_materials++;
    } else {
      std::cout << "Deallocating Material	at ";
      std::cout << print_pos(x, y) << std::endl;
      int success = deallocate(material_arena, mat);
      if (success == -1) {
        assert(false);
      }
    }
  }
}

void ChunkHandler::move_material(Chunk *chunk, Material *material) {
  Material* colliding_material = nullptr;
  vector2 old_pos = material->position;
  vector2 new_pos = element_updater.pos_update(&old_pos, &material->velocity, &colliding_material);


  if(colliding_material != nullptr)
  {
    element_updater.solid_collision_resolver(material, colliding_material, new_pos, 0.5f, 10.0f);
  }

  assert(material != nullptr);
  assert(in_world(new_pos.x, new_pos.y));
  assert(material->position.x == old_pos.x && material->position.y == old_pos.y);

  //std::cout << "Attempting to move material at " << print_pos(old_pos) << " to " << print_pos(new_pos) << '\n';
  if (get_material(new_pos.x, new_pos.y) != nullptr) {
    material->frames_rest ++;
    //std::cout << "Move invalid\n";
    return;
  }
  else {
    material->frames_rest = 0;
  }

  Neighbors neighbors = get_neighbors(old_pos, new_pos);

  if(neighbors.org_up != nullptr && is_asleep(neighbors.org_up))
  {
    set_active(neighbors.org_up);
  }
  if(neighbors.org_left != nullptr && is_asleep(neighbors.org_left))
  {
    set_active(neighbors.org_left);
  }
  if(neighbors.org_right != nullptr && is_asleep(neighbors.org_right))
  {
    set_active(neighbors.org_right);
  }
  
  /*
  if(neighbors.new_down != nullptr && is_asleep(neighbors.new_down))
  {
    set_active(neighbors.new_down);
  }
  */

  Chunk *new_chunk = get_chunk(new_pos.x, new_pos.y);

  if (new_chunk == nullptr) {
    new_chunk = init_chunk(new_pos.x, new_pos.y);
    assert(chunk->grid[index(old_pos.x, old_pos.y)] != nullptr);
    chunk->grid[index(old_pos.x, old_pos.y)] = nullptr;
    chunk->num_materials--;
    assert(*chunk->active_list.find(material) == material);
    chunk->active_list.erase(material);
    new_chunk->grid[index(new_pos.x, new_pos.y)] = material;
    new_chunk->active_list.insert(material);
    new_chunk->num_materials++;
    material->position.x = new_pos.x;
    material->position.y = new_pos.y;
  } else if (new_chunk == chunk) {
    assert(*chunk->active_list.find(material) == material);
    assert(chunk->grid[index(old_pos.x, old_pos.y)] != nullptr);
    assert(chunk->grid[index(new_pos.x, new_pos.y)] == nullptr);
    chunk->grid[index(new_pos.x, new_pos.y)] = material;
    chunk->grid[index(old_pos.x, old_pos.y)] = nullptr;
    material->position.x = new_pos.x;
    material->position.y = new_pos.y;
  } else if (new_chunk != chunk) {
    assert(chunk->grid[index(old_pos.x, old_pos.y)] != nullptr);
    assert(new_chunk->grid[index(new_pos.x, new_pos.y)] == nullptr);
    chunk->grid[index(old_pos.x, old_pos.y)] = nullptr;
    assert(*chunk->active_list.find(material) == material);
    chunk->num_materials--;
    chunk->active_list.erase(material);
    new_chunk->grid[index(new_pos.x, new_pos.y)] = material;
    new_chunk->active_list.insert(material);
    new_chunk->num_materials++;
    material->position.x = new_pos.x;
    material->position.y = new_pos.y;
  }
}

void ChunkHandler::commit_changes() {

  std::shuffle(move_list.begin(), move_list.end(), shuffler);

  for (size_t i = 0; i < move_list.size(); i++) {
    Material* mat = move_list[i];
    move_material(get_chunk(mat->position.x, mat->position.y), mat);
  }

  Animation *animation;
  std::vector<Material *> clear_list;
  for (auto i = animation_list.begin(); i != animation_list.end(); i++) {
    animation = &(i->second);
    i->first->tex_offset = animation->color_change;
    animation->frames--;
    if (animation->frames == 0) {
      clear_list.push_back(i->first);
    }
  }

  move_list.clear();
  for (size_t i = 0; i < clear_list.size(); i++) {
    animation_list.erase(clear_list[i]);
  }

}

void ChunkHandler::draw_chunks_to_texture(Renderer *render, bool debug_mode) {
  std::vector<float> tex_offsets;
  std::vector<glm::mat4> transforms;
  std::vector<glm::mat4> debug_rect_transforms;
  std::vector<glm::mat4> chunk_rect_transforms;

  static vector2 size{1, 1};

  for (size_t i = 0; i < iter_chunks.size(); i++) {
    Chunk *chunk = iter_chunks[i];

    if (debug_mode) {
      glm::mat4 chunk_model = glm::mat4(1.0f);
      glm::mat4 dirty_model = glm::mat4(1.0f);

      vector2 start_coords{chunk->coords.x * chunk_width,
        chunk->coords.y * chunk_height};
      vector2 end_coords;
      end_coords.x = (chunk->coords.x * chunk_width) + chunk_width;
      end_coords.y = (chunk->coords.y * chunk_height) + chunk_height;

      chunk_model = glm::translate(
          chunk_model, glm::vec3(start_coords.x, start_coords.y, 0.0f));
      chunk_model =
        glm::scale(chunk_model, glm::vec3(chunk_width, chunk_height, 1.0f));

      if (chunk->lower_rect_coords.x != -1) {
        dirty_model = glm::translate(
            dirty_model, glm::vec3(chunk->upper_rect_coords.x,
              chunk->upper_rect_coords.y, 0.0f));
        dirty_model = glm::scale(
            dirty_model,
            glm::vec3((chunk->lower_rect_coords.x - chunk->upper_rect_coords.x),
              (chunk->lower_rect_coords.y - chunk->upper_rect_coords.y),
              1.0f));
      }

      chunk_rect_transforms.push_back(chunk_model);
      debug_rect_transforms.push_back(dirty_model);
    }

    for (auto j = chunk->active_list.begin(); j != chunk->active_list.end();
        j++) {
      tex_offsets.push_back((*j)->tex_offset);

      glm::mat4 model = glm::mat4(1.0f);

      model = glm::translate(
          model, glm::vec3((*j)->position.x, (*j)->position.y, 0.0f));
      model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));
      transforms.push_back(model);
    }

    for (auto j = chunk->asleep_list.begin(); j != chunk->asleep_list.end();
        j++) {
      tex_offsets.push_back((*j)->tex_offset);

      glm::mat4 model = glm::mat4(1.0f);

      model = glm::translate(
          model, glm::vec3((*j)->position.x, (*j)->position.y, 0.0f));
      model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));
      transforms.push_back(model);
    }
  }

  render->draw_materials(tex_offsets, transforms, size);
  render->draw_empty_rect(debug_rect_transforms, tex_coords.DIRTY_DEBUG);
  render->draw_empty_rect(chunk_rect_transforms, tex_coords.GRID_DEBUG);
}

ChunkHandler::~ChunkHandler() {
  assert(free_arena(material_arena) == 0);
  move_list.clear();
  swap_list.clear();
  chunks.clear();
  for (int i = 0; i < iter_chunks.size(); i++) {
    delete[] iter_chunks[i]->grid;
    iter_chunks[i]->active_list.clear();
  }
}
