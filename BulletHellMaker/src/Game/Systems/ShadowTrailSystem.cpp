#include <Game/Systems/ShadowTrailSystem.h>

#include <memory>

#include <Game/EntityCreationQueue.h>

ShadowTrailSystem::ShadowTrailSystem(EntityCreationQueue& queue, entt::DefaultRegistry& registry) 
	: queue(queue), registry(registry) {
}

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