#include "SpriteAnimationSystem.h"
#include "Components.h"
#include <SFML/Graphics.hpp>

void SpriteAnimationSystem::update(float deltaTime) {
	auto view = registry.view<SpriteComponent>();

	view.each([&](auto entity, auto& sprite) {
		sprite.update(deltaTime);
	});
}
