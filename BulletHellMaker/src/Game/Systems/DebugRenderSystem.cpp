#include <Game/Systems/DebugRenderSystem.h>

#include <Constants.h>
#include <Game/Components/SpriteComponent.h>
#include <Game/Components/PlayerTag.h>

DebugRenderSystem::DebugRenderSystem(entt::DefaultRegistry& registry, sf::RenderWindow& window, SpriteLoader& spriteLoader, float resolutionMultiplier) : RenderSystem(registry, window, spriteLoader, resolutionMultiplier, false) {
	circleFormat.setFillColor(sf::Color(sf::Color::Transparent));
	circleFormat.setOutlineThickness(3);
}

void DebugRenderSystem::update(float deltaTime) {
	for (int i = 0; i < layers.size(); i++) {
		layers[i].clear();
	}

	auto view = registry.view<PositionComponent, SpriteComponent>(entt::persistent_t{});
	view.each([this](auto entity, auto& position, auto& sprite) {
		if (sprite.getSprite()) {
			sprite.getSprite()->setPosition(position.getX() * resolutionMultiplier, (MAP_HEIGHT - position.getY()) * resolutionMultiplier);
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
	backgroundTempLayerTexture.clear(sf::Color::Transparent);
	backgroundSprite.setPosition(0, -MAP_HEIGHT * resolutionMultiplier);
	backgroundTempLayerTexture.draw(backgroundSprite);
	backgroundTempLayerTexture.display();
	sf::Sprite backgroundAsSprite(backgroundTempLayerTexture.getTexture());
	sf::RenderStates backgroundStates;
	backgroundStates.blendMode = DEFAULT_BLEND_MODE;
	window.draw(backgroundAsSprite, backgroundStates);

	// Draw the layers onto the window directly
	for (int i = 0; i < layers.size(); i++) {
		for (SpriteComponent& sprite : layers[i]) {
			std::shared_ptr<sf::Sprite> spritePtr = sprite.getSprite();

			window.draw(*spritePtr);
		}
	}

	// Draw the hitboxes
	auto view2 = registry.view<PositionComponent, HitboxComponent>(entt::persistent_t{});
	view2.each([this](auto entity, auto position, auto hitbox) {
		// Radius takes into account outline thickness expanding outwards,
		// making the circle appear larger than it actually is
		if (hitbox.getRadius() > 0) {
			circleFormat.setRadius(hitbox.getRadius() - circleFormat.getOutlineThickness());
			if (registry.attachee<PlayerTag>() == entity) {
				circleFormat.setOutlineColor(sf::Color(sf::Color::Blue));
			} else {
				circleFormat.setOutlineColor(sf::Color(sf::Color::Magenta));
			}
			circleFormat.setPosition((position.getX() + hitbox.getX() - circleFormat.getRadius()) * resolutionMultiplier, (MAP_HEIGHT - (position.getY() + hitbox.getY() + circleFormat.getRadius())) * resolutionMultiplier);
			window.draw(circleFormat);
		}
	});
}

void DebugRenderSystem::setResolution(SpriteLoader& spriteLoader, float resolutionMultiplier) {
	spriteLoader.setGlobalSpriteScale(resolutionMultiplier);

	int newPlayAreaWidth = std::lrint(MAP_WIDTH * resolutionMultiplier);
	int newPlayAreaHeight = std::lrint(MAP_HEIGHT * resolutionMultiplier);

	sf::View view(sf::FloatRect(0, -newPlayAreaHeight, newPlayAreaWidth, newPlayAreaHeight));
	backgroundTempLayerTexture.create(newPlayAreaWidth, newPlayAreaHeight);
	backgroundTempLayerTexture.setView(view);

	backgroundSprite.setScale(MAP_WIDTH / backgroundTextureWidth * resolutionMultiplier, MAP_HEIGHT / backgroundTextureHeight * resolutionMultiplier);

	if (onResolutionChange) {
		onResolutionChange->publish();
	}
}
