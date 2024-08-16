#ifndef UPDATER_H
#define UPDATER_H
#include <GameState.hpp>
#include <vector>
#include <PoolArena.hpp>

Material* create_material(MatType material, MaterialState* mat_state, PoolArena* arena,  glm::vec2 pos);
void update_materials(MaterialState* state, std::vector<std::vector<Material*>>& grid);
void update_down();
void update_side();
void update_down_side();

#endif
