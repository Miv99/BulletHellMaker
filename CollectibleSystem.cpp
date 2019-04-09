#include "CollectibleSystem.h"

void CollectibleSystem::update(float deltaTime) {
	auto view = registry.view<PositionComponent, HitboxComponent, CollectibleComponent>();

	view.each([&](auto entity, auto& pos, auto& hitbox, auto& collectible) {
		collectible.update(queue, registry, entity, pos, hitbox);
	});
}