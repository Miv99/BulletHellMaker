#include "RenderSystem.h"
#include <algorithm>

void RenderSystem::update(float deltaTime) {
	for (std::pair<int, std::vector<std::reference_wrapper<SpriteComponent>>>& pair : layers) {
		pair.second.clear();
	}

	auto view = registry.view<PositionComponent, SpriteComponent>(entt::persistent_t{});
	view.each([&](auto entity, auto& position, auto& sprite) {
		sprite.getSprite()->setPosition(position.getX(), -position.getY());
		layers[sprite.getRenderLayer()].second.push_back(std::ref(sprite));
	});

	for (std::pair<int, std::vector<std::reference_wrapper<SpriteComponent>>>& pair : layers) {
		std::sort(pair.second.begin(), pair.second.end(), SubLayerComparator());
	}

	for (std::pair<int, std::vector<std::reference_wrapper<SpriteComponent>>>& layer : layers) {
		for (SpriteComponent& sprite : layer.second) {
			std::shared_ptr<sf::Sprite> spritePtr = sprite.getSprite();

			if (sprite.usesShader()) {
				window.draw(*spritePtr, &sprite.getShader());
			} else {
				window.draw(*spritePtr);
			}
		}
	}
}
