#include <Game/Systems/RenderSystem/RenderSystem.h>

#include <algorithm>
#include <string>
#include <cmath>

#include <LevelPack/Level.h>

const sf::BlendMode RenderSystem::DEFAULT_BLEND_MODE = sf::BlendMode(sf::BlendMode::Factor::SrcAlpha, sf::BlendMode::Factor::OneMinusSrcAlpha, sf::BlendMode::Equation::Add, sf::BlendMode::Factor::SrcAlpha, sf::BlendMode::Factor::OneMinusSrcAlpha, sf::BlendMode::Equation::Add);

RenderSystem::RenderSystem(entt::DefaultRegistry & registry, sf::RenderWindow & window, SpriteLoader& spriteLoader, float resolutionMultiplier, bool initShaders) : registry(registry), window(window), resolutionMultiplier(resolutionMultiplier) {
	// Initialize layers to be size of the max layer
	layers = std::vector<std::vector<std::reference_wrapper<SpriteComponent>>>(HIGHEST_RENDER_LAYER + 1);

	setResolution(spriteLoader, resolutionMultiplier);

	blurEffect.setBlendMode(DEFAULT_BLEND_MODE);
}

void RenderSystem::update(float deltaTime) {
	for (int i = 0; i < layers.size(); i++) {
		layers[i].clear();
		layerTextures[i].clear(sf::Color::Transparent);
	}

	auto view = registry.view<PositionComponent, SpriteComponent>(entt::persistent_t{});
	view.each([&](auto entity, auto& position, auto& sprite) {
		if (sprite.getSprite()) {
			sprite.getSprite()->setPosition(position.getX() * resolutionMultiplier, -position.getY() * resolutionMultiplier);
			layers[sprite.getRenderLayer()].push_back(std::ref(sprite));
		}
	});

	for (std::vector<std::reference_wrapper<SpriteComponent>>& layer : layers) {
		std::sort(layer.begin(), layer.end(), SubLayerComparator());
	}

	// Move background
	backgroundX = std::fmod((backgroundX + backgroundScrollSpeedX * deltaTime), backgroundTextureSizeX);
	backgroundY = std::fmod((backgroundY + backgroundScrollSpeedY * deltaTime), backgroundTextureSizeY);
	backgroundSprite.setTextureRect(sf::IntRect(backgroundX, backgroundY, backgroundTextureWidth, backgroundTextureHeight));

	// Draw background by drawing onto the temp layer first to limit the visible part of the background to the play area
	backgroundSprite.setPosition(0, -MAP_HEIGHT * resolutionMultiplier);
	backgroundTempLayerTexture.draw(backgroundSprite);
	backgroundTempLayerTexture.display();
	sf::Sprite backgroundAsSprite(backgroundTempLayerTexture.getTexture());
	sf::RenderStates backgroundStates;
	backgroundStates.blendMode = DEFAULT_BLEND_MODE;
	window.draw(backgroundAsSprite, backgroundStates);

	for (int i = 0; i < layers.size(); i++) {
		for (SpriteComponent& sprite : layers[i]) {
			std::shared_ptr<sf::Sprite> spritePtr = sprite.getSprite();

			if (sprite.usesShader()) {
				layerTextures[i].draw(*spritePtr, &sprite.getShader());
			} else {
				layerTextures[i].draw(*spritePtr);
			}
		}
		if (layers[i].size() == 0) {
			continue;
		}
		layerTextures[i].display();

		if (i == SHADOW_LAYER) {
			// If shadow layer, blur the layer
			blurEffect.apply(layerTextures[i], window);
		} else {
			// Draw layer normally
			sf::Sprite textureAsSprite(layerTextures[i].getTexture());
			sf::RenderStates states;
			states.blendMode = DEFAULT_BLEND_MODE;
			window.draw(textureAsSprite, states);
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
	backgroundTempLayerTexture.create(newPlayAreaWidth, newPlayAreaHeight);
	backgroundTempLayerTexture.setView(view);

	backgroundSprite.setScale(MAP_WIDTH / backgroundTextureWidth * resolutionMultiplier, MAP_HEIGHT / backgroundTextureHeight * resolutionMultiplier);

	if (onResolutionChange) {
		onResolutionChange->publish();
	}
}

void RenderSystem::setBackground(std::shared_ptr<sf::Texture> background) {
	this->background = background;
	backgroundTextureSizeX = background->getSize().x;
	backgroundTextureSizeY = background->getSize().y;
	// Start the initial background view such that the bottom-left corner of the view is
	// the bottom-left corner of the background texture
	backgroundX = 0;
	backgroundY = backgroundTextureSizeY - MAP_HEIGHT;

	// Create the sprite
	backgroundSprite.setTexture(*(this->background));
}

sf::Vector2u RenderSystem::getResolution() {
	return backgroundTempLayerTexture.getSize();
}

std::shared_ptr<entt::SigH<void()>> RenderSystem::getOnResolutionChange() {
	if (!onResolutionChange) {
		onResolutionChange = std::make_shared<entt::SigH<void()>>();
	}
	return onResolutionChange;
}