#include "SpriteAnimationSystem.h"
#include "Components.h"
#include <SFML/Graphics.hpp>

void SpriteAnimationSystem::update(float deltaTime) {
	auto view = registry.view<SpriteComponent>();
	auto animatableSetView = registry.view<PositionComponent, AnimatableSetComponent, SpriteComponent>(entt::persistent_t{});

	animatableSetView.each([&](auto entity, auto& position, auto& set, auto& sprite) {
		set.update(spriteLoader, position.getX(), position.getY(), sprite, deltaTime);
	});

	view.each([&](auto entity, auto& sprite) {
		sprite.update(deltaTime);
	});
}
