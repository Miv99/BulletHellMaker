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

	// Initialize layer textures
	for (int i = 0; i < highestLayer + 1; i++) {
		sf::View view = window.getView();
		layerTextures[i].create(view.getSize().x - (MAP_HEIGHT - MAP_WIDTH), view.getSize().y);
		view.setSize(MAP_WIDTH, MAP_HEIGHT);
		view.setCenter(view.getSize().x/2.0f, -view.getSize().y/2.0f);
		layerTextures[i].setView(view);
	}
	sf::View view = window.getView();
	tempLayerTexture.create(view.getSize().x - (MAP_HEIGHT - MAP_WIDTH), view.getSize().y);
	view.setSize(MAP_WIDTH, MAP_HEIGHT);
	view.setCenter(view.getSize().x / 2.0f, -view.getSize().y / 2.0f);
	tempLayerTexture.setView(view);

	// Calculate sprite scaling
	spriteHorizontalScale = view.getSize().x / tempLayerTexture.getSize().x;
	spriteVerticalScale = view.getSize().y / tempLayerTexture.getSize().y;

	// Initialize global shaders
	std::unique_ptr<sf::Shader> shadowBlurHorizontal = std::make_unique<sf::Shader>();
	if (!shadowBlurHorizontal->loadFromFile("Shaders/shadow_blur.frag", sf::Shader::Fragment)) {
		throw "Could not load Shaders/shadow_blur.frag";
	}
	shadowBlurHorizontal->setUniform("texture", sf::Shader::CurrentTexture);
	shadowBlurHorizontal->setUniform("resolution", sf::Vector2f(layerTextures[SHADOW_LAYER].getSize().x, layerTextures[SHADOW_LAYER].getSize().y));
	shadowBlurHorizontal->setUniform("direction", sf::Vector2f(1, 0));
	globalShaders[SHADOW_LAYER].push_back(std::move(shadowBlurHorizontal));
	
	std::unique_ptr<sf::Shader> shadowBlurVertical = std::make_unique<sf::Shader>();
	if (!shadowBlurVertical->loadFromFile("Shaders/shadow_blur.frag", sf::Shader::Fragment)) {
		throw "Could not load Shaders/shadow_blur.frag";
	}
	shadowBlurVertical->setUniform("texture", sf::Shader::CurrentTexture);
	shadowBlurVertical->setUniform("resolution", sf::Vector2f(layerTextures[SHADOW_LAYER].getSize().x, layerTextures[SHADOW_LAYER].getSize().y));
	shadowBlurVertical->setUniform("direction", sf::Vector2f(0, 1));
	globalShaders[SHADOW_LAYER].push_back(std::move(shadowBlurVertical));
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
		layerTextures[i].display();
		
		if (globalShaders.count(i) > 0) {
			if (globalShaders[i].size() == 1) {
				sf::Sprite textureAsSprite(sf::Sprite(layerTextures[i].getTexture()));
				textureAsSprite.setPosition(0, -(float)layerTextures[i].getSize().y);
				window.draw(textureAsSprite, &(*globalShaders[i][0]));
			} else {
				bool alt = false;
				for (int a = 0; a < globalShaders[i].size(); a++) {
					alt = !alt;
					if (alt) {
						sf::Sprite textureAsSprite(sf::Sprite(layerTextures[i].getTexture()));
						textureAsSprite.setScale(spriteHorizontalScale, spriteVerticalScale);
						textureAsSprite.setPosition(0, -(float)layerTextures[i].getSize().y);

						tempLayerTexture.clear(sf::Color::Transparent);
						sf::RenderStates states;
						states.shader = &(*globalShaders[i][a]);
						states.blendMode = sf::BlendAdd;
						tempLayerTexture.draw(textureAsSprite, states);
						tempLayerTexture.display();
					} else {
						sf::Sprite textureAsSprite(sf::Sprite(tempLayerTexture.getTexture()));
						textureAsSprite.setScale(spriteHorizontalScale, spriteVerticalScale);
						textureAsSprite.setPosition(0, -(float)layerTextures[i].getSize().y);

						layerTextures[i].clear(sf::Color::Transparent);
						sf::RenderStates states;
						states.shader = &(*globalShaders[i][a]);
						states.blendMode = sf::BlendAdd;
						layerTextures[i].draw(textureAsSprite, states);
						layerTextures[i].display();
					}
				}
				if (alt) {
					sf::Sprite textureAsSprite(sf::Sprite(tempLayerTexture.getTexture()));
					textureAsSprite.setPosition(0, -(float)layerTextures[i].getSize().y);
					window.draw(textureAsSprite);
				} else {
					sf::Sprite textureAsSprite(sf::Sprite(layerTextures[i].getTexture()));
					textureAsSprite.setPosition(0, -(float)layerTextures[i].getSize().y);
					window.draw(textureAsSprite);
				}
			}
		} else {
			sf::Sprite textureAsSprite(sf::Sprite(layerTextures[i].getTexture()));
			textureAsSprite.setPosition(0, -(float)layerTextures[i].getSize().y);
			window.draw(textureAsSprite);
		}
	}
}
