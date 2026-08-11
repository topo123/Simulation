#include <iostream>
#include <Material.hpp>
#include <ChunkHandler.hpp>
#include <Elements.hpp>
#include <PoolArena.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace Catch::Matchers;

ChunkHandler handler;
PoolArena* arena;
Elements* updater;

std::ostream& operator<<(std::ostream& os, const fvector2& p) {
	return os << "(" << p.x << ", " << p.y << ")";
}

std::ostream& operator<<(std::ostream& os, const vector2& p) {
	return os << "(" << p.x << ", " << p.y << ")";
}

void set_material_attributes(Material* mat, vector2 position, vector2 velocity, MatType material_type)
{
	mat->position.x = position.x;
	mat->position.y = position.y;
	
	mat->velocity.x = velocity.x;
	mat->velocity.y = velocity.y;

	mat->mat_id = material_type;
}

void init_test()
{
	arena = init_pool(800 * 800, sizeof(Material));
	handler.init_chunk_handler(10, 10, 800, 800, arena);
	updater = &handler.element_updater;
}

void set_material_props(Material* material, vector2 position, fvector2 velocity)
{
	material->position = position;
	material->velocity = velocity;
}

TEST_CASE("Bresenham Move Test", "[bresenham]")
{
	std::vector<Material*> materials;
	init_test();

	vector2 initial_pos {0, 0};
	fvector2 velocity{0, 10.0f};
	vector2 final_pos{0 , 10};

	Material* colliding_material;

	updater->pos_update(&initial_pos, &velocity, &colliding_material);

	REQUIRE(final_pos.x == 0);
	REQUIRE(final_pos.y == 10);
}


TEST_CASE("Bresenham Collision Test", "[bresenham]")
{
	std::vector<Material*> materials;
	init_test();

	Material* mat1 = static_cast<Material*>(allocate(arena));
	Material* mat2 = static_cast<Material*>(allocate(arena));

	set_material_attributes(mat1, {0, 0}, {0, 10}, MatType::SAND);
	set_material_attributes(mat2, {0, 5}, {0, 0}, MatType::SAND);

	materials.push_back(mat1);
	materials.push_back(mat2);

	handler.add_materials(materials);

	handler.update_chunk(handler.add_materials_list[0], 0.5);


	REQUIRE(mat1->position.x == mat2->position.x);
	REQUIRE(mat1->position.y == mat2->position.y - 1);

	free_arena(arena);
	materials.clear();
}

