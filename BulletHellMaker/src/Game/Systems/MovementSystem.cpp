#include <Game/Systems/MovementSystem.h>

#include <cmath>

#include <Game/Components/PositionComponent.h>
#include <Game/Components/MovementPathComponent.h>
#include <Game/Components/HitboxComponent.h>
#include <Game/Components/SpriteComponent.h>
#include <Game/Components/EMPSpawnerComponent.h>
#include <Game/EntityCreationQueue.h>

MovementSystem::MovementSystem(EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry& registry)
	: queue(queue), spriteLoader(spriteLoader), registry(registry) {
}

void MovementSystem::update(float deltaTime) {
	auto view = registry.view<PositionComponent, MovementPathComponent>(entt::persistent_t{});
	view.each([this, deltaTime](auto entity, auto& position, auto& path) {
		float prevX = position.getX();
		float prevY = position.getY();
		path.update(queue, registry, entity, position, deltaTime);
		// Calculate angle of movement
		float angle = std::atan2(position.getY() - prevY, position.getX() - prevX);
		if (registry.has<SpriteComponent>(entity)) {
			// Rotate sprite
			auto& sprite = registry.get<SpriteComponent>(entity);
			sprite.rotate(angle);

			if (registry.has<HitboxComponent>(entity)) {
				if (sprite.getSprite()) {
					// Rotate hitbox according to sprite orientation
					registry.get<HitboxComponent>(entity).rotate(sprite.getSprite());
				} else {
					// Rotate hitbox according to angle
					registry.get<HitboxComponent>(entity).rotate(angle);
				}
			}
		} else {
			// Rotate hitbox
			if (registry.has<HitboxComponent>(entity)) {
				registry.get<HitboxComponent>(entity).rotate(angle);
			}
		}
	});

	auto spawnerView = registry.view<EMPSpawnerComponent>();
	spawnerView.each([this, deltaTime](auto entity, auto& spawner) {
		spawner.update(registry, spriteLoader, queue, deltaTime);
	});
}
