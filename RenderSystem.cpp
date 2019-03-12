#include "RenderSystem.h"
#include <iostream>

void RenderSystem::update(float deltaTime) {
	auto view = registry.view<PositionComponent, SpriteComponent>(entt::persistent_t{});

	view.each([&](auto entity, auto& position, auto& sprite) {
		auto spritePtr = sprite.getSprite();
		spritePtr->setPosition(position.getX(), position.getY());

		if (sprite.usesShader()) {
			window.draw(*spritePtr, &sprite.getShader());
		}
		else {
			window.draw(*spritePtr);
		}
	});
}
