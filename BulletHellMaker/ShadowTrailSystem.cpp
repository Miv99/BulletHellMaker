#include "ShadowTrailSystem.h"
#include <memory>

void ShadowTrailSystem::update(float deltaTime) {
	auto view = registry.view<PositionComponent, SpriteComponent, ShadowTrailComponent>(entt::persistent_t{});

	view.each([&](auto entity, auto& position, auto& sprite, auto& trail) {
		if (trail.update(deltaTime)) {
			auto spritePtr = sprite.getSprite();
			if (spritePtr) {
				queue.pushBack(std::make_unique<SpawnShadowTrailCommand>(registry, *spritePtr, position.getX(), position.getY(), sprite.getInheritedRotationAngle(), trail.getLifespan()));
			}
		}
	});
}