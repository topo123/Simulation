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

TEST_CASE("Velocity collisions", "[velocity]")
{
	std::vector<Material*> materials;
	init_test();

	vector2 inital {10, 1};
	fvector2 vec {-1.0, 0.0f};
	vector2 final = updater->pos_update(&inital, &vec, nullptr);

	REQUIRE(final.x == 9);
	REQUIRE(final.y == 1);

}

