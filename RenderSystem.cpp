#include "RenderSystem.h"

void RenderSystem::update(float deltaTime) {
	auto view = registry.view<PositionComponent, SpriteComponent>(entt::persistent_t{});

	view.each([&](auto entity, auto& position, auto& sprite) {
		std::shared_ptr<sf::Sprite> spritePtr = sprite.getSprite();
		spritePtr->setPosition(position.getX(), position.getY());

		if (sprite.usesShader()) {
			window.draw(*spritePtr, &sprite.getShader());
		}
		else {
			window.draw(*spritePtr);
		}
	});
}
