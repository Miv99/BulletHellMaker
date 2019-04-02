#include "MovementSystem.h"

void MovementSystem::update(float deltaTime) {
	auto view = registry.view<PositionComponent, MovementPathComponent>(entt::persistent_t{});
	view.each([&](auto entity, auto& position, auto& path) {
		path.update(queue, registry, entity, position, deltaTime);
	});

	auto spawnerView = registry.view<EMPSpawnerComponent>();
	spawnerView.each([&](auto entity, auto& spawner) {
		spawner.update(registry, spriteLoader, queue, deltaTime);
	});
}
