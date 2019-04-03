#include "MovementSystem.h"
#include <cmath>

void MovementSystem::update(float deltaTime) {
	auto view = registry.view<PositionComponent, MovementPathComponent>(entt::persistent_t{});
	view.each([&](auto entity, auto& position, auto& path) {
		float prevX = position.getX();
		float prevY = position.getY();
		path.update(queue, registry, entity, position, deltaTime);
		// Calculate angle of movement
		float angle = std::atan2(position.getY() - prevY, position.getX() - prevX);
		// Rotate hitbox
		if (registry.has<HitboxComponent>(entity)) {
			registry.get<HitboxComponent>(entity).rotate(angle);
		}
		// Rotate sprite
		if (registry.has<SpriteComponent>(entity)) {
			registry.get<SpriteComponent>(entity).rotate(angle);
		}
	});

	auto spawnerView = registry.view<EMPSpawnerComponent>();
	spawnerView.each([&](auto entity, auto& spawner) {
		spawner.update(registry, spriteLoader, queue, deltaTime);
	});
}
