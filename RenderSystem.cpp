#include "RenderSystem.h"
#include <algorithm>

RenderSystem::RenderSystem(entt::DefaultRegistry & registry, sf::RenderWindow & window) : registry(registry), window(window) {
	// Initialize layers to be size of the max layer
	int highestLayer = ENEMY_BULLET_LAYER;
	layers = std::vector<std::pair<int, std::vector<std::reference_wrapper<SpriteComponent>>>>(highestLayer + 1);
	layers[SHADOW_LAYER] = std::make_pair(SHADOW_LAYER, std::vector<std::reference_wrapper<SpriteComponent>>());
	layers[PLAYER_BULLET_LAYER] = std::make_pair(PLAYER_BULLET_LAYER, std::vector<std::reference_wrapper<SpriteComponent>>());
	layers[ENEMY_LAYER] = std::make_pair(ENEMY_LAYER, std::vector<std::reference_wrapper<SpriteComponent>>());
	layers[PLAYER_LAYER] = std::make_pair(PLAYER_LAYER, std::vector<std::reference_wrapper<SpriteComponent>>());
	layers[ITEM_LAYER] = std::make_pair(ITEM_LAYER, std::vector<std::reference_wrapper<SpriteComponent>>());
	layers[ENEMY_BULLET_LAYER] = std::make_pair(ENEMY_BULLET_LAYER, std::vector<std::reference_wrapper<SpriteComponent>>());

	for (int i = 0; i < highestLayer + 1; i++) {
		sf::View view = window.getView();
		layerTextures[i].create(view.getSize().x - (MAP_HEIGHT - MAP_WIDTH), view.getSize().y);
		view.setSize(MAP_WIDTH, MAP_HEIGHT);
		view.setCenter(view.getSize().x/2.0f, -view.getSize().y/2.0f);
		layerTextures[i].setView(view);
	}
}

void RenderSystem::update(float deltaTime) {
	for (int i = 0; i < layers.size(); i++) {
		layers[i].second.clear();
		layerTextures[i].clear(sf::Color::Transparent);
	}

	auto view = registry.view<PositionComponent, SpriteComponent>(entt::persistent_t{});
	view.each([&](auto entity, auto& position, auto& sprite) {
		sprite.getSprite()->setPosition(position.getX(), -position.getY());
		layers[sprite.getRenderLayer()].second.push_back(std::ref(sprite));
	});

	for (std::pair<int, std::vector<std::reference_wrapper<SpriteComponent>>>& pair : layers) {
		std::sort(pair.second.begin(), pair.second.end(), SubLayerComparator());
	}

	for (int i = 0; i < layers.size(); i++) {
		for (SpriteComponent& sprite : layers[i].second) {
			std::shared_ptr<sf::Sprite> spritePtr = sprite.getSprite();

			if (sprite.usesShader()) {
				layerTextures[i].draw(*spritePtr, &sprite.getShader());
			} else {
				layerTextures[i].draw(*spritePtr);
			}

		}
		sf::Sprite textureAsSprite(sf::Sprite(layerTextures[i].getTexture()));
		textureAsSprite.setPosition(0, 0);
		textureAsSprite.setRotation(180);
		textureAsSprite.setScale(-1, 1);
		//textureAsSprite.setScale(1, -1);
		window.draw(textureAsSprite);
	}
}
