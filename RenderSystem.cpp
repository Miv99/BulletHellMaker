#include "RenderSystem.h"
#include <algorithm>
#include <string>
#include <cmath>
#include "Level.h"

RenderSystem::RenderSystem(entt::DefaultRegistry & registry, sf::RenderWindow & window, SpriteLoader& spriteLoader, float resolutionMultiplier, bool initShaders) : registry(registry), window(window), resolutionMultiplier(resolutionMultiplier) {
	// Initialize layers to be size of the max layer
	layers = std::vector<std::pair<int, std::vector<std::reference_wrapper<SpriteComponent>>>>(HIGHEST_RENDER_LAYER + 1);
	layers[SHADOW_LAYER] = std::make_pair(SHADOW_LAYER, std::vector<std::reference_wrapper<SpriteComponent>>());
	layers[PLAYER_BULLET_LAYER] = std::make_pair(PLAYER_BULLET_LAYER, std::vector<std::reference_wrapper<SpriteComponent>>());
	layers[ENEMY_LAYER] = std::make_pair(ENEMY_LAYER, std::vector<std::reference_wrapper<SpriteComponent>>());
	layers[ENEMY_BOSS_LAYER] = std::make_pair(ENEMY_BOSS_LAYER, std::vector<std::reference_wrapper<SpriteComponent>>());
	layers[PLAYER_LAYER] = std::make_pair(PLAYER_LAYER, std::vector<std::reference_wrapper<SpriteComponent>>());
	layers[ITEM_LAYER] = std::make_pair(ITEM_LAYER, std::vector<std::reference_wrapper<SpriteComponent>>());
	layers[ENEMY_BULLET_LAYER] = std::make_pair(ENEMY_BULLET_LAYER, std::vector<std::reference_wrapper<SpriteComponent>>());

	setResolution(spriteLoader, resolutionMultiplier);

	// Initialize global shaders
	if (initShaders) {
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
}

void RenderSystem::update(float deltaTime) {
	for (int i = 0; i < layers.size(); i++) {
		layers[i].second.clear();
		layerTextures[i].clear(sf::Color::Transparent);
	}

	auto view = registry.view<PositionComponent, SpriteComponent>(entt::persistent_t{});
	view.each([&](auto entity, auto& position, auto& sprite) {
		if (sprite.getSprite()) {
			sprite.getSprite()->setPosition(position.getX() * resolutionMultiplier, -position.getY() * resolutionMultiplier);
			layers[sprite.getRenderLayer()].second.push_back(std::ref(sprite));
		}
	});

	for (std::pair<int, std::vector<std::reference_wrapper<SpriteComponent>>>& pair : layers) {
		std::sort(pair.second.begin(), pair.second.end(), SubLayerComparator());
	}

	// Move background
	backgroundX = std::fmod((backgroundX + backgroundScrollSpeedX * deltaTime), backgroundTextureSizeX);
	backgroundY = std::fmod((backgroundY + backgroundScrollSpeedY * deltaTime), backgroundTextureSizeY);
	backgroundSprite.setTextureRect(sf::IntRect(backgroundX, backgroundY, backgroundTextureWidth, backgroundTextureHeight));

	// Draw background by drawing onto the temp layer first to limit the visible part of the background to the play area
	tempLayerTexture.clear(sf::Color::Transparent);

	backgroundSprite.setPosition(0, -MAP_HEIGHT * resolutionMultiplier);
	tempLayerTexture.draw(backgroundSprite);
	tempLayerTexture.display();
	sf::Sprite backgroundAsSprite(tempLayerTexture.getTexture());
	sf::RenderStates backgroundStates;
	backgroundStates.blendMode = DEFAULT_BLEND_MODE;
	window.draw(backgroundAsSprite, backgroundStates);

	for (int i = 0; i < layers.size(); i++) {
		for (SpriteComponent& sprite : layers[i].second) {
			std::shared_ptr<sf::Sprite> spritePtr = sprite.getSprite();

			if (sprite.usesShader()) {
				layerTextures[i].draw(*spritePtr, &sprite.getShader());
			}
			else {
				layerTextures[i].draw(*spritePtr);
			}
		}
		if (layers[i].second.size() == 0) {
			continue;
		}
		layerTextures[i].display();

		if (bloom[i].usesBloom()) {
			if (globalShaders.count(i) > 0) {
				bool alt = false;
				for (int a = 0; a < globalShaders[i].size(); a++) {
					alt = !alt;
					if (alt) {
						sf::Sprite textureAsSprite(layerTextures[i].getTexture());
						// idk why this is needed but it is
						textureAsSprite.setScale(spriteHorizontalScale, spriteVerticalScale);

						tempLayerTexture.clear(sf::Color::Transparent);
						sf::RenderStates states;
						states.shader = &(*globalShaders[i][a]);
						states.blendMode = DEFAULT_BLEND_MODE;
						tempLayerTexture.draw(textureAsSprite, states);
						tempLayerTexture.display();
					}
					else {
						sf::Sprite textureAsSprite(tempLayerTexture.getTexture());
						// idk why this is needed but it is
						textureAsSprite.setScale(spriteHorizontalScale, spriteVerticalScale);

						layerTextures[i].clear(sf::Color::Transparent);
						sf::RenderStates states;
						states.shader = &(*globalShaders[i][a]);
						states.blendMode = DEFAULT_BLEND_MODE;
						layerTextures[i].draw(textureAsSprite, states);
						layerTextures[i].display();
					}
				}

				if (alt) {
					sf::Texture nonblurredTexture = sf::Texture(tempLayerTexture.getTexture());
					bloomGlowShader.setUniform("strength", bloom[i].getGlowStrength());

					// Apply blur shaders
					alt = false;
					for (int a = 0; a < bloomBlurShaders.size() + 1; a++) {
						sf::Shader* shader;
						auto mode = sf::BlendAdd;
						if (a == 0) {
							shader = &bloomDarkShader;
							shader->setUniform("minBright", bloom[i].getMinBright());
							mode = DEFAULT_BLEND_MODE;
						}
						else {
							shader = &*bloomBlurShaders[a - 1];
						}

						alt = !alt;
						if (alt) {
							sf::Sprite textureAsSprite(layerTextures[i].getTexture());
							// idk why this is needed but it is
							textureAsSprite.setScale(spriteHorizontalScale, spriteVerticalScale);

							tempLayerTexture.clear(sf::Color::Transparent);
							sf::RenderStates states;
							states.shader = shader;
							states.blendMode = mode;
							tempLayerTexture.draw(textureAsSprite, states);
							tempLayerTexture.display();
						}
						else {
							sf::Sprite textureAsSprite(tempLayerTexture.getTexture());
							// idk why this is needed but it is
							textureAsSprite.setScale(spriteHorizontalScale, spriteVerticalScale);

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
						sf::RenderStates states;
						states.shader = &bloomGlowShader;
						states.blendMode = sf::BlendAdd;
						window.draw(textureAsSprite, states);
					}
					else {
						sf::Sprite textureAsSprite(layerTextures[i].getTexture());

						sf::RenderStates states;
						states.shader = &bloomGlowShader;
						states.blendMode = sf::BlendAdd;
						window.draw(textureAsSprite, states);
					}
					// draw nonblurred onto window
					sf::Sprite textureAsSprite2(nonblurredTexture);
					sf::RenderStates states;
					states.shader = &bloomGlowShader;
					states.blendMode = bloom[i].getBlendMode();
					window.draw(textureAsSprite2, states);
				}
				else {
					sf::Texture nonblurredTexture = sf::Texture(layerTextures[i].getTexture());
					bloomGlowShader.setUniform("strength", bloom[i].getGlowStrength());

					// Apply blur shaders
					alt = false;
					for (int a = 0; a < bloomBlurShaders.size() + 1; a++) {
						sf::Shader* shader;
						auto mode = sf::BlendAdd;
						if (a == 0) {
							shader = &bloomDarkShader;
							shader->setUniform("minBright", bloom[i].getMinBright());
							mode = DEFAULT_BLEND_MODE;
						}
						else {
							shader = &*bloomBlurShaders[a - 1];
						}

						alt = !alt;
						if (alt) {
							sf::Sprite textureAsSprite(layerTextures[i].getTexture());
							// idk why this is needed but it is
							textureAsSprite.setScale(spriteHorizontalScale, spriteVerticalScale);

							tempLayerTexture.clear(sf::Color::Transparent);
							sf::RenderStates states;
							states.shader = shader;
							states.blendMode = mode;
							tempLayerTexture.draw(textureAsSprite, states);
							tempLayerTexture.display();
						}
						else {
							sf::Sprite textureAsSprite(tempLayerTexture.getTexture());
							// idk why this is needed but it is
							textureAsSprite.setScale(spriteHorizontalScale, spriteVerticalScale);

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
						sf::RenderStates states;
						states.shader = &bloomGlowShader;
						states.blendMode = sf::BlendAdd;
						window.draw(textureAsSprite, states);
					}
					else {
						sf::Sprite textureAsSprite(layerTextures[i].getTexture());

						sf::RenderStates states;
						states.shader = &bloomGlowShader;
						states.blendMode = sf::BlendAdd;
						window.draw(textureAsSprite, states);
					}
					// draw nonblurred onto window
					sf::Sprite textureAsSprite2(nonblurredTexture);
					sf::RenderStates states;
					states.shader = &bloomGlowShader;
					states.blendMode = bloom[i].getBlendMode();
					window.draw(textureAsSprite2, states);
				}
			}
			else {
				sf::Texture nonblurredTexture = sf::Texture(layerTextures[i].getTexture());
				bloomGlowShader.setUniform("strength", bloom[i].getGlowStrength());

				// Apply blur shaders
				bool alt = false;
				for (int a = 0; a < bloomBlurShaders.size() + 1; a++) {
					sf::Shader* shader;
					auto mode = sf::BlendAdd;
					if (a == 0) {
						shader = &bloomDarkShader;
						shader->setUniform("minBright", bloom[i].getMinBright());
						mode = DEFAULT_BLEND_MODE;
					}
					else {
						shader = &*bloomBlurShaders[a - 1];
					}

					alt = !alt;
					if (alt) {
						sf::Sprite textureAsSprite(layerTextures[i].getTexture());
						// idk why this is needed but it is
						textureAsSprite.setScale(spriteHorizontalScale, spriteVerticalScale);

						tempLayerTexture.clear(sf::Color::Transparent);
						sf::RenderStates states;
						states.shader = shader;
						states.blendMode = mode;
						tempLayerTexture.draw(textureAsSprite, states);
						tempLayerTexture.display();
					}
					else {
						sf::Sprite textureAsSprite(tempLayerTexture.getTexture());
						// idk why this is needed but it is
						textureAsSprite.setScale(spriteHorizontalScale, spriteVerticalScale);

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

					sf::RenderStates states;
					states.shader = &bloomGlowShader;
					states.blendMode = sf::BlendAdd;
					window.draw(textureAsSprite, states);
				}
				else {
					sf::Sprite textureAsSprite(layerTextures[i].getTexture());

					sf::RenderStates states;
					states.shader = &bloomGlowShader;
					states.blendMode = sf::BlendAdd;
					window.draw(textureAsSprite, states);
				}
				// draw nonblurred onto window
				sf::Sprite textureAsSprite2(nonblurredTexture);
				sf::RenderStates states;
				states.shader = &bloomGlowShader;
				states.blendMode = bloom[i].getBlendMode();
				window.draw(textureAsSprite2, states);
			}
		}
		else {
			if (globalShaders.count(i) > 0) {
				if (globalShaders[i].size() == 1) {
					sf::Sprite textureAsSprite(layerTextures[i].getTexture());

					sf::RenderStates states;
					states.shader = &(*globalShaders[i][0]);
					states.blendMode = DEFAULT_BLEND_MODE;
					window.draw(textureAsSprite, states);
				}
				else {
					bool alt = false;
					for (int a = 0; a < globalShaders[i].size(); a++) {
						alt = !alt;
						if (alt) {
							sf::Sprite textureAsSprite(layerTextures[i].getTexture());
							// idk why this is needed but it is
							textureAsSprite.setScale(spriteHorizontalScale, spriteVerticalScale);

							tempLayerTexture.clear(sf::Color::Transparent);
							sf::RenderStates states;
							states.shader = &(*globalShaders[i][a]);
							states.blendMode = DEFAULT_BLEND_MODE;
							tempLayerTexture.draw(textureAsSprite, states);
							tempLayerTexture.display();
						}
						else {
							sf::Sprite textureAsSprite(tempLayerTexture.getTexture());
							// idk why this is needed but it is
							textureAsSprite.setScale(spriteHorizontalScale, spriteVerticalScale);

							layerTextures[i].clear(sf::Color::Transparent);
							sf::RenderStates states;
							states.shader = &(*globalShaders[i][a]);
							states.blendMode = DEFAULT_BLEND_MODE;
							layerTextures[i].draw(textureAsSprite, states);
							layerTextures[i].display();
						}
					}
					if (alt) {
						sf::Sprite textureAsSprite(tempLayerTexture.getTexture());
						window.draw(textureAsSprite);
					}
					else {
						sf::Sprite textureAsSprite(layerTextures[i].getTexture());

						sf::RenderStates states;
						states.blendMode = DEFAULT_BLEND_MODE;
						window.draw(textureAsSprite, states);
					}
				}
			}
			else {
				sf::Sprite textureAsSprite(layerTextures[i].getTexture());

				sf::RenderStates states;
				states.blendMode = DEFAULT_BLEND_MODE;
				window.draw(textureAsSprite, states);
			}
		}
	}
}

void RenderSystem::setResolution(SpriteLoader& spriteLoader, float resolutionMultiplier) {
	spriteLoader.setGlobalSpriteScale(resolutionMultiplier);

	int newPlayAreaWidth = (int)std::round(MAP_WIDTH * resolutionMultiplier);
	int newPlayAreaHeight = (int)std::round(MAP_HEIGHT * resolutionMultiplier);

	sf::View view(sf::FloatRect(0, -newPlayAreaHeight, newPlayAreaWidth, newPlayAreaHeight));
	for (int i = 0; i < HIGHEST_RENDER_LAYER + 1; i++) {
		layerTextures[i].create(newPlayAreaWidth, newPlayAreaHeight);
		layerTextures[i].setView(view);
	}
	tempLayerTexture.create(newPlayAreaWidth, newPlayAreaHeight);
	tempLayerTexture.setView(view);
	tempLayerTexture2.create(newPlayAreaWidth, newPlayAreaHeight);
	tempLayerTexture2.setView(view);

	spriteHorizontalScale = view.getSize().x / tempLayerTexture.getSize().x;
	spriteVerticalScale = view.getSize().y / tempLayerTexture.getSize().y;

	for (auto it = globalShaders.begin(); it != globalShaders.end(); it++) {
		for (auto& shader : it->second) {
			shader->setUniform("resolution", sf::Vector2f(newPlayAreaWidth, newPlayAreaHeight));
		}
	}
	for (auto& shader : bloomBlurShaders) {
		shader->setUniform("resolution", sf::Vector2f(newPlayAreaWidth, newPlayAreaHeight));
	}

	backgroundSprite.setScale(MAP_WIDTH / backgroundTextureWidth * resolutionMultiplier, MAP_HEIGHT / backgroundTextureHeight * resolutionMultiplier);

	if (onResolutionChange) {
		onResolutionChange->publish();
	}
}

void RenderSystem::loadLevelRenderSettings(std::shared_ptr<Level> level) {
	if (level) {
		bloom = std::vector<BloomSettings>(HIGHEST_RENDER_LAYER + 1, BloomSettings());
		auto levelBloomSettings = level->getBloomLayerSettings();
		for (int i = 0; i < levelBloomSettings.size(); i++) {
			bloom[i] = levelBloomSettings[i];
		}
	} else {
		bloom = std::vector<BloomSettings>(HIGHEST_RENDER_LAYER + 1, BloomSettings());
		BloomSettings bloomSettings;
		bloomSettings.setUsesBloom(false);
		for (int i = 0; i < bloom.size(); i++) {
			bloom[i] = bloomSettings;
		}
	}
}

void RenderSystem::setBackground(sf::Texture background) {
	this->background = background;
	backgroundTextureSizeX = background.getSize().x;
	backgroundTextureSizeY = background.getSize().y;
	// Start the initial background view such that the bottom-left corner of the view is
	// the bottom-left corner of the background texture
	backgroundX = 0;
	backgroundY = backgroundTextureSizeY - MAP_HEIGHT;

	// Create the sprite
	backgroundSprite.setTexture(this->background);
}

sf::Vector2u RenderSystem::getResolution() {
	return tempLayerTexture.getSize();
}

std::shared_ptr<entt::SigH<void()>> RenderSystem::getOnResolutionChange() {
	if (!onResolutionChange) {
		onResolutionChange = std::make_shared<entt::SigH<void()>>();
	}
	return onResolutionChange;
}

std::string BloomSettings::format() const {
	return formatBool(useBloom) + tos(glowStrength) + tos(minBright) + tos(static_cast<int>(blendMode.colorSrcFactor))
		+ tos(static_cast<int>(blendMode.colorDstFactor)) + tos(static_cast<int>(blendMode.colorEquation))
		+ tos(static_cast<int>(blendMode.alphaSrcFactor)) + tos(static_cast<int>(blendMode.alphaDstFactor))
		+ tos(static_cast<int>(blendMode.alphaEquation));
}

void BloomSettings::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	if (std::stoi(items[0]) == 1) {
		useBloom = true;
	}
	else {
		useBloom = false;
	}
	glowStrength = std::stof(items[1]);
	minBright = std::stof(items[2]);
	blendMode.colorSrcFactor = static_cast<sf::BlendMode::Factor>(std::stoi(items[3]));
	blendMode.colorDstFactor = static_cast<sf::BlendMode::Factor>(std::stoi(items[4]));
	blendMode.colorEquation = static_cast<sf::BlendMode::Equation>(std::stoi(items[5]));
	blendMode.alphaSrcFactor = static_cast<sf::BlendMode::Factor>(std::stoi(items[6]));
	blendMode.alphaDstFactor = static_cast<sf::BlendMode::Factor>(std::stoi(items[7]));
	blendMode.alphaEquation = static_cast<sf::BlendMode::Equation>(std::stoi(items[8]));
}