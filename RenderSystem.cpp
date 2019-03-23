#include "RenderSystem.h"
#include <iostream>

void RenderSystem::update(float deltaTime) {
	auto view = registry.view<PositionComponent, SpriteComponent>(entt::persistent_t{});

	view.each([&](auto entity, auto& position, auto& sprite) {
		std::shared_ptr<sf::Sprite> spritePtr = sprite.getSprite();
		spritePtr->setPosition(position.getX(), position.getY());

		if (sprite.usesShader()) {
			window.draw(*spritePtr, &sprite.getShader());
		}
		else {
			std::cout << (*spritePtr).getTextureRect().width << ", " << (*spritePtr).getTextureRect().height << std::endl;
			window.draw(*spritePtr);
		}
	});
}
