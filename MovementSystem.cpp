#include "MovementSystem.h"
#include <iostream>

void MovementSystem::update(float deltaTime) {
	auto view = registry.view<PositionComponent, MovementPathComponent>(entt::persistent_t{});
	view.each([&](auto entity, auto& position, auto& path) {
		sf::Vector2f newPos = path.update(queue, registry, entity, deltaTime);
		position.setPosition(newPos);
	});

	auto spawnerView = registry.view<EMPSpawnerComponent>();
	spawnerView.each([&](auto entity, auto& spawner) {
		spawner.update(registry, spriteLoader, queue, deltaTime);
	});

	//TODO: player movement stuff somehow

	elapsedTime += deltaTime;
}
