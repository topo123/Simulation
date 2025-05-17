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

	Material m1;
	Material m2;
	Material m3;

	set_material_props(&m1, {100, 100}, {0.0f, 5.0f});
	set_material_props(&m2, {100, 105}, {0.0f, 0.0f});
	set_material_props(&m3, {101, 109}, {0.0f, 0.0f});

	materials.push_back(&m1);
	materials.push_back(&m2);



	handler.add_materials(materials);

	fvector2 m1_vel = m1.velocity;
	vector2 new_pos = updater->solid_physics_update(&m1, 1.0f);
	m1.position = new_pos;

	vector2 comp_pos {100, 104};

	fvector2 new_velocity {m1_vel.y/5.0f, m1_vel.y - m1_vel.y/10.0f};

	REQUIRE(new_pos == comp_pos);

	REQUIRE_THAT(std::abs(m1.velocity.x), WithinAbs(new_velocity.x, 0.01)); 
	REQUIRE_THAT(m1.velocity.y, WithinAbs(std::abs(new_velocity.y), 0.01)); 

	new_pos = updater->solid_physics_update(&m1, 1.0f);

	std::cout << "New move: " << new_pos << std::endl;

	REQUIRE(new_pos.x == -1); 
	REQUIRE(new_pos.y == -1);

	std::cout << m1.velocity << std::endl;
}

