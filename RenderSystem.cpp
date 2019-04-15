#include "RenderSystem.h"
#include <algorithm>
#include <string>

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

	bloom = std::vector<BloomSettings>(highestLayer + 1, BloomSettings{ false });
	bloom[PLAYER_LAYER] = BloomSettings{ true, 1.3f, 0.0f, blendMode };
	bloom[ENEMY_LAYER] = BloomSettings{ true, 1.3f, 0.0f, blendMode };
	bloom[PLAYER_BULLET_LAYER] = BloomSettings{ true, 1.2f, 0.05f, blendMode };
	bloom[ENEMY_BULLET_LAYER] = BloomSettings{ true, 1.2f, 0.05f, blendMode };

	// Initialize layer textures
	for (int i = 0; i < highestLayer + 1; i++) {
		sf::View view = window.getView();
		layerTextures[i].create(view.getSize().x - (MAP_HEIGHT - MAP_WIDTH), view.getSize().y);
		view.setSize(MAP_WIDTH, MAP_HEIGHT);
		view.setCenter(view.getSize().x / 2.0f, -view.getSize().y / 2.0f);
		layerTextures[i].setView(view);
	}
	sf::View view = window.getView();
	tempLayerTexture.create(view.getSize().x - (MAP_HEIGHT - MAP_WIDTH), view.getSize().y);
	view.setSize(MAP_WIDTH, MAP_HEIGHT);
	view.setCenter(view.getSize().x / 2.0f, -view.getSize().y / 2.0f);
	tempLayerTexture.setView(view);
	tempLayerTexture2.create(view.getSize().x - (MAP_HEIGHT - MAP_WIDTH), view.getSize().y);
	tempLayerTexture2.setView(view);

	// Calculate sprite scaling
	spriteHorizontalScale = view.getSize().x / tempLayerTexture.getSize().x;
	spriteVerticalScale = view.getSize().y / tempLayerTexture.getSize().y;

	// Initialize global shaders

	// Shadow shaders
	{
		std::unique_ptr<sf::Shader> shadowBlurHorizontal = std::make_unique<sf::Shader>();
		if (!shadowBlurHorizontal->loadFromFile("Shaders/gaussian_blur.frag", sf::Shader::Fragment)) {
			throw "Could not load Shaders/gaussian_blur.frag";
		}
		shadowBlurHorizontal->setUniform("texture", sf::Shader::CurrentTexture);
		shadowBlurHorizontal->setUniform("resolution", sf::Vector2f(layerTextures[SHADOW_LAYER].getSize().x, layerTextures[SHADOW_LAYER].getSize().y));
		shadowBlurHorizontal->setUniform("direction", sf::Vector2f(1, 0));
		shadowBlurHorizontal->setUniform("mode", 0);
		globalShaders[SHADOW_LAYER].push_back(std::move(shadowBlurHorizontal));

		std::unique_ptr<sf::Shader> shadowBlurVertical = std::make_unique<sf::Shader>();
		if (!shadowBlurVertical->loadFromFile("Shaders/gaussian_blur.frag", sf::Shader::Fragment)) {
			throw "Could not load Shaders/gaussian_blur.frag";
		}
		shadowBlurVertical->setUniform("texture", sf::Shader::CurrentTexture);
		shadowBlurVertical->setUniform("resolution", sf::Vector2f(layerTextures[SHADOW_LAYER].getSize().x, layerTextures[SHADOW_LAYER].getSize().y));
		shadowBlurVertical->setUniform("direction", sf::Vector2f(0, 1));
		shadowBlurVertical->setUniform("mode", 0);
		globalShaders[SHADOW_LAYER].push_back(std::move(shadowBlurVertical));
	}

	// Bloom blur shaders
	{
		std::unique_ptr<sf::Shader> blurHorizontal = std::make_unique<sf::Shader>();
		if (!blurHorizontal->loadFromFile("Shaders/n_linear_blur.frag", sf::Shader::Fragment)) {
			throw "Could not load Shaders/n_linear_blur.frag";
		}
		blurHorizontal->setUniform("texture", sf::Shader::CurrentTexture);
		blurHorizontal->setUniform("resolution", sf::Vector2f(layerTextures[SHADOW_LAYER].getSize().x, layerTextures[SHADOW_LAYER].getSize().y));
		blurHorizontal->setUniform("direction", sf::Vector2f(1, 0));
		blurHorizontal->setUniform("n", 15);
		bloomBlurShaders.push_back(std::move(blurHorizontal));

		std::unique_ptr<sf::Shader> blurVertical = std::make_unique<sf::Shader>();
		if (!blurVertical->loadFromFile("Shaders/n_linear_blur.frag", sf::Shader::Fragment)) {
			throw "Could not load Shaders/n_linear_blur.frag";
		}
		blurVertical->setUniform("texture", sf::Shader::CurrentTexture);
		blurVertical->setUniform("resolution", sf::Vector2f(layerTextures[SHADOW_LAYER].getSize().x, layerTextures[SHADOW_LAYER].getSize().y));
		blurVertical->setUniform("direction", sf::Vector2f(0, 1));
		blurVertical->setUniform("n", 15);
		bloomBlurShaders.push_back(std::move(blurVertical));
	}

	// Bloom bright shader
	if (!bloomDarkShader.loadFromFile("Shaders/bright.frag", sf::Shader::Fragment)) {
		throw "Could not load Shaders/bright.frag";
	}
	bloomDarkShader.setUniform("texture", sf::Shader::CurrentTexture);

	// Bloom glow shader
	if (!bloomGlowShader.loadFromFile("Shaders/glow.frag", sf::Shader::Fragment)) {
		throw "Could not load Shaders/glow.frag";
	}
	bloomGlowShader.setUniform("texture", sf::Shader::CurrentTexture);
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

	// Background
	/*
	tempLayerTexture.clear(sf::Color(0, 0, 0, 255));
	sf::Sprite aaa(sf::Sprite(tempLayerTexture.getTexture()));
	aaa.setPosition(0, -(float)tempLayerTexture.getSize().y);
	sf::RenderStates asd;
	asd.blendMode = blendMode;
	window.draw(aaa, asd);
	*/

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

		if (bloom[i].useBloom) {
			if (globalShaders.count(i) > 0) {
				bool alt = false;
				for (int a = 0; a < globalShaders[i].size(); a++) {
					alt = !alt;
					if (alt) {
						sf::Sprite textureAsSprite(layerTextures[i].getTexture());
						// idk why this is needed but it is
						textureAsSprite.setScale(spriteHorizontalScale, spriteVerticalScale);
						textureAsSprite.setPosition(0, -(float)layerTextures[i].getSize().y);

						tempLayerTexture.clear(sf::Color::Transparent);
						sf::RenderStates states;
						states.shader = &(*globalShaders[i][a]);
						states.blendMode = blendMode;
						tempLayerTexture.draw(textureAsSprite, states);
						tempLayerTexture.display();
					} else {
						sf::Sprite textureAsSprite(tempLayerTexture.getTexture());
						// idk why this is needed but it is
						textureAsSprite.setScale(spriteHorizontalScale, spriteVerticalScale);
						textureAsSprite.setPosition(0, -(float)layerTextures[i].getSize().y);

						layerTextures[i].clear(sf::Color::Transparent);
						sf::RenderStates states;
						states.shader = &(*globalShaders[i][a]);
						states.blendMode = blendMode;
						layerTextures[i].draw(textureAsSprite, states);
						layerTextures[i].display();
					}
				}

				if (alt) {
					sf::Texture nonblurredTexture = sf::Texture(tempLayerTexture.getTexture());
					bloomGlowShader.setUniform("strength", bloom[i].glowStrength);

					// Apply blur shaders
					alt = false;
					for (int a = 0; a < bloomBlurShaders.size() + 1; a++) {
						sf::Shader* shader;
						auto mode = sf::BlendAdd;
						if (a == 0) {
							shader = &bloomDarkShader;
							shader->setUniform("minBright", bloom[i].minBright);
							mode = blendMode;
						} else {
							shader = &*bloomBlurShaders[a - 1];
						}

						alt = !alt;
						if (alt) {
							sf::Sprite textureAsSprite(layerTextures[i].getTexture());
							// idk why this is needed but it is
							textureAsSprite.setScale(spriteHorizontalScale, spriteVerticalScale);
							textureAsSprite.setPosition(0, -(float)layerTextures[i].getSize().y);

							tempLayerTexture.clear(sf::Color::Transparent);
							sf::RenderStates states;
							states.shader = shader;
							states.blendMode = mode;
							tempLayerTexture.draw(textureAsSprite, states);
							tempLayerTexture.display();
						} else {
							sf::Sprite textureAsSprite(tempLayerTexture.getTexture());
							// idk why this is needed but it is
							textureAsSprite.setScale(spriteHorizontalScale, spriteVerticalScale);
							textureAsSprite.setPosition(0, -(float)layerTextures[i].getSize().y);

							layerTextures[i].clear(sf::Color::Transparent);
							sf::RenderStates states;
							states.shader = shader;
							states.blendMode = mode;
							layerTextures[i].draw(textureAsSprite, states);
							layerTextures[i].display();
						}
					}

					// draw blurred onto window
					if (alt) {
						sf::Sprite textureAsSprite(tempLayerTexture.getTexture());
						textureAsSprite.setPosition(0, -(float)tempLayerTexture.getSize().y);
						sf::RenderStates states;
						states.shader = &bloomGlowShader;
						states.blendMode = sf::BlendAdd;
						window.draw(textureAsSprite, states);
					} else {
						sf::Sprite textureAsSprite(layerTextures[i].getTexture());
						textureAsSprite.setPosition(0, -(float)layerTextures[i].getSize().y);

						sf::RenderStates states;
						states.shader = &bloomGlowShader;
						states.blendMode = sf::BlendAdd;
						window.draw(textureAsSprite, states);
					}
					// draw nonblurred onto window
					sf::Sprite textureAsSprite2(nonblurredTexture);
					textureAsSprite2.setPosition(0, -(float)tempLayerTexture.getSize().y);
					sf::RenderStates states;
					states.shader = &bloomGlowShader;
					states.blendMode = bloom[i].blendMode;
					window.draw(textureAsSprite2, states);
				} else {
					sf::Texture nonblurredTexture = sf::Texture(layerTextures[i].getTexture());
					bloomGlowShader.setUniform("strength", bloom[i].glowStrength);

					// Apply blur shaders
					alt = false;
					for (int a = 0; a < bloomBlurShaders.size() + 1; a++) {
						sf::Shader* shader;
						auto mode = sf::BlendAdd;
						if (a == 0) {
							shader = &bloomDarkShader;
							shader->setUniform("minBright", bloom[i].minBright);
							mode = blendMode;
						} else {
							shader = &*bloomBlurShaders[a - 1];
						}

						alt = !alt;
						if (alt) {
							sf::Sprite textureAsSprite(layerTextures[i].getTexture());
							// idk why this is needed but it is
							textureAsSprite.setScale(spriteHorizontalScale, spriteVerticalScale);
							textureAsSprite.setPosition(0, -(float)layerTextures[i].getSize().y);

							tempLayerTexture.clear(sf::Color::Transparent);
							sf::RenderStates states;
							states.shader = shader;
							states.blendMode = mode;
							tempLayerTexture.draw(textureAsSprite, states);
							tempLayerTexture.display();
						} else {
							sf::Sprite textureAsSprite(tempLayerTexture.getTexture());
							// idk why this is needed but it is
							textureAsSprite.setScale(spriteHorizontalScale, spriteVerticalScale);
							textureAsSprite.setPosition(0, -(float)layerTextures[i].getSize().y);

							layerTextures[i].clear(sf::Color::Transparent);
							sf::RenderStates states;
							states.shader = shader;
							states.blendMode = mode;
							layerTextures[i].draw(textureAsSprite, states);
							layerTextures[i].display();
						}
					}

					// draw blurred onto window
					if (alt) {
						sf::Sprite textureAsSprite(tempLayerTexture.getTexture());
						textureAsSprite.setPosition(0, -(float)tempLayerTexture.getSize().y);
						sf::RenderStates states;
						states.shader = &bloomGlowShader;
						states.blendMode = sf::BlendAdd;
						window.draw(textureAsSprite, states);
					} else {
						sf::Sprite textureAsSprite(layerTextures[i].getTexture());
						textureAsSprite.setPosition(0, -(float)layerTextures[i].getSize().y);

						sf::RenderStates states;
						states.shader = &bloomGlowShader;
						states.blendMode = sf::BlendAdd;
						window.draw(textureAsSprite, states);
					}
					// draw nonblurred onto window
					sf::Sprite textureAsSprite2(nonblurredTexture);
					textureAsSprite2.setPosition(0, -(float)tempLayerTexture.getSize().y);
					sf::RenderStates states;
					states.shader = &bloomGlowShader;
					states.blendMode = bloom[i].blendMode;
					window.draw(textureAsSprite2, states);
				}
			} else {
				sf::Texture nonblurredTexture = sf::Texture(layerTextures[i].getTexture());
				bloomGlowShader.setUniform("strength", bloom[i].glowStrength);

				// Apply blur shaders
				bool alt = false;
				for (int a = 0; a < bloomBlurShaders.size() + 1; a++) {
					sf::Shader* shader;
					auto mode = sf::BlendAdd;
					if (a == 0) {
						shader = &bloomDarkShader;
						shader->setUniform("minBright", bloom[i].minBright);
						mode = blendMode;
					} else {
						shader = &*bloomBlurShaders[a - 1];
					}

					alt = !alt;
					if (alt) {
						sf::Sprite textureAsSprite(layerTextures[i].getTexture());
						// idk why this is needed but it is
						textureAsSprite.setScale(spriteHorizontalScale, spriteVerticalScale);
						textureAsSprite.setPosition(0, -(float)layerTextures[i].getSize().y);

						tempLayerTexture.clear(sf::Color::Transparent);
						sf::RenderStates states;
						states.shader = shader;
						states.blendMode = mode;
						tempLayerTexture.draw(textureAsSprite, states);
						tempLayerTexture.display();
					} else {
						sf::Sprite textureAsSprite(tempLayerTexture.getTexture());
						// idk why this is needed but it is
						textureAsSprite.setScale(spriteHorizontalScale, spriteVerticalScale);
						textureAsSprite.setPosition(0, -(float)tempLayerTexture.getSize().y);

						layerTextures[i].clear(sf::Color::Transparent);
						sf::RenderStates states;
						states.shader = shader;
						states.blendMode = mode;
						layerTextures[i].draw(textureAsSprite, states);
						layerTextures[i].display();
					}
				}

				// draw blurred onto window
				if (alt) {
					sf::Sprite textureAsSprite(tempLayerTexture.getTexture());
					textureAsSprite.setPosition(0, -(float)tempLayerTexture.getSize().y);

					sf::RenderStates states;
					states.shader = &bloomGlowShader;
					states.blendMode = sf::BlendAdd;
					window.draw(textureAsSprite, states);
				} else {
					sf::Sprite textureAsSprite(layerTextures[i].getTexture());
					textureAsSprite.setPosition(0, -(float)layerTextures[i].getSize().y);

					sf::RenderStates states;
					states.shader = &bloomGlowShader;
					states.blendMode = sf::BlendAdd;
					window.draw(textureAsSprite, states);
				}
				// draw nonblurred onto window
				sf::Sprite textureAsSprite2(nonblurredTexture);
				textureAsSprite2.setPosition(0, -(float)tempLayerTexture.getSize().y);
				sf::RenderStates states;
				states.shader = &bloomGlowShader;
				states.blendMode = bloom[i].blendMode;
				window.draw(textureAsSprite2, states);
			}
		} else {
			if (globalShaders.count(i) > 0) {
				if (globalShaders[i].size() == 1) {
					sf::Sprite textureAsSprite(layerTextures[i].getTexture());
					textureAsSprite.setPosition(0, -(float)layerTextures[i].getSize().y);

					sf::RenderStates states;
					states.shader = &(*globalShaders[i][0]);
					states.blendMode = blendMode;
					window.draw(textureAsSprite, states);
				} else {
					bool alt = false;
					for (int a = 0; a < globalShaders[i].size(); a++) {
						alt = !alt;
						if (alt) {
							sf::Sprite textureAsSprite(layerTextures[i].getTexture());
							// idk why this is needed but it is
							textureAsSprite.setScale(spriteHorizontalScale, spriteVerticalScale);
							textureAsSprite.setPosition(0, -(float)layerTextures[i].getSize().y);

							tempLayerTexture.clear(sf::Color::Transparent);
							sf::RenderStates states;
							states.shader = &(*globalShaders[i][a]);
							states.blendMode = blendMode;
							tempLayerTexture.draw(textureAsSprite, states);
							tempLayerTexture.display();
						} else {
							sf::Sprite textureAsSprite(tempLayerTexture.getTexture());
							// idk why this is needed but it is
							textureAsSprite.setScale(spriteHorizontalScale, spriteVerticalScale);
							textureAsSprite.setPosition(0, -(float)layerTextures[i].getSize().y);

							layerTextures[i].clear(sf::Color::Transparent);
							sf::RenderStates states;
							states.shader = &(*globalShaders[i][a]);
							states.blendMode = blendMode;
							layerTextures[i].draw(textureAsSprite, states);
							layerTextures[i].display();
						}
					}
					if (alt) {
						sf::Sprite textureAsSprite(tempLayerTexture.getTexture());
						textureAsSprite.setPosition(0, -(float)layerTextures[i].getSize().y);
						window.draw(textureAsSprite);
					} else {
						sf::Sprite textureAsSprite(layerTextures[i].getTexture());
						textureAsSprite.setPosition(0, -(float)layerTextures[i].getSize().y);

						sf::RenderStates states;
						states.blendMode = blendMode;
						window.draw(textureAsSprite, states);
					}
				}
			} else {
				sf::Sprite textureAsSprite(layerTextures[i].getTexture());
				textureAsSprite.setPosition(0, -(float)layerTextures[i].getSize().y);

				sf::RenderStates states;
				states.blendMode = blendMode;
				window.draw(textureAsSprite, states);
			}
		}
	}
}
