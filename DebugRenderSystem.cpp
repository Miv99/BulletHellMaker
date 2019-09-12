#include "DebugRenderSystem.h"
#include "Components.h"
#include "Constants.h"

static sf::BlendMode DEFAULT_BLEND_MODE = sf::BlendMode(sf::BlendMode::Factor::SrcAlpha, sf::BlendMode::Factor::OneMinusSrcAlpha, sf::BlendMode::Equation::Add, sf::BlendMode::Factor::SrcAlpha, sf::BlendMode::Factor::OneMinusSrcAlpha, sf::BlendMode::Equation::Add);

DebugRenderSystem::DebugRenderSystem(entt::DefaultRegistry& registry, sf::RenderWindow& window, float playAreaWidth, float playAreaHeight) : registry(registry), window(window) {
	circleFormat.setFillColor(sf::Color(sf::Color::Transparent));
	circleFormat.setOutlineThickness(3);

	setResolution(playAreaWidth, playAreaHeight);

	layers = std::vector<std::pair<int, std::vector<std::reference_wrapper<SpriteComponent>>>>(HIGHEST_RENDER_LAYER + 1);
	layers[SHADOW_LAYER] = std::make_pair(SHADOW_LAYER, std::vector<std::reference_wrapper<SpriteComponent>>());
	layers[PLAYER_BULLET_LAYER] = std::make_pair(PLAYER_BULLET_LAYER, std::vector<std::reference_wrapper<SpriteComponent>>());
	layers[ENEMY_LAYER] = std::make_pair(ENEMY_LAYER, std::vector<std::reference_wrapper<SpriteComponent>>());
	layers[ENEMY_BOSS_LAYER] = std::make_pair(ENEMY_BOSS_LAYER, std::vector<std::reference_wrapper<SpriteComponent>>());
	layers[PLAYER_LAYER] = std::make_pair(PLAYER_LAYER, std::vector<std::reference_wrapper<SpriteComponent>>());
	layers[ITEM_LAYER] = std::make_pair(ITEM_LAYER, std::vector<std::reference_wrapper<SpriteComponent>>());
	layers[ENEMY_BULLET_LAYER] = std::make_pair(ENEMY_BULLET_LAYER, std::vector<std::reference_wrapper<SpriteComponent>>());
}

void DebugRenderSystem::update(float deltaTime) {
	for (int i = 0; i < layers.size(); i++) {
		layers[i].second.clear();
	}

	auto view = registry.view<PositionComponent, SpriteComponent>(entt::persistent_t{});
	view.each([&](auto entity, auto& position, auto& sprite) {
		sprite.getSprite()->setPosition(position.getX(), -position.getY());
		layers[sprite.getRenderLayer()].second.push_back(std::ref(sprite));
	});

	for (std::pair<int, std::vector<std::reference_wrapper<SpriteComponent>>>& pair : layers) {
		std::sort(pair.second.begin(), pair.second.end(), SubLayerComparator());
	}

	// Move background
	backgroundX = std::fmod((backgroundX + backgroundScrollSpeedX * deltaTime), backgroundTextureSizeX);
	backgroundY = std::fmod((backgroundY + backgroundScrollSpeedY * deltaTime), backgroundTextureSizeY);
	backgroundSprite.setTextureRect(sf::IntRect(backgroundX, backgroundY, MAP_WIDTH, MAP_HEIGHT));

	// Draw background by drawing onto the temp layer first to limit the visible part of the background to the play area
	tempLayerTexture.clear(sf::Color::Transparent);
	backgroundSprite.setPosition(0, -MAP_HEIGHT);
	tempLayerTexture.draw(backgroundSprite);
	tempLayerTexture.display();
	sf::Sprite backgroundAsSprite(tempLayerTexture.getTexture());
	backgroundAsSprite.setPosition(0, -(float)tempLayerTexture.getSize().y);
	sf::RenderStates backgroundStates;
	backgroundStates.blendMode = DEFAULT_BLEND_MODE;
	window.draw(backgroundAsSprite, backgroundStates);

	// Draw the layers onto the window directly
	for (int i = 0; i < layers.size(); i++) {
		for (SpriteComponent& sprite : layers[i].second) {
			std::shared_ptr<sf::Sprite> spritePtr = sprite.getSprite();

			window.draw(*spritePtr);
		}
	}

	// Draw the hitboxes
	auto view2 = registry.view<PositionComponent, HitboxComponent>(entt::persistent_t{});
	view2.each([&](auto entity, auto position, auto hitbox) {
		// Radius takes into account outline thickness expanding outwards,
		// making the circle appear larger than it actually is
		circleFormat.setRadius(hitbox.getRadius() - circleFormat.getOutlineThickness());
		if (registry.attachee<PlayerTag>() == entity) {
			circleFormat.setOutlineColor(sf::Color(sf::Color::Blue));
		} else {
			circleFormat.setOutlineColor(sf::Color(sf::Color::Magenta));
		}
		circleFormat.setPosition(position.getX() + hitbox.getX() - circleFormat.getRadius(), -(position.getY() + hitbox.getY() + circleFormat.getRadius()));
		window.draw(circleFormat);
	});
}

void DebugRenderSystem::setResolution(int newPlayAreaWidth, int newPlayAreaHeight) {
	sf::View view(sf::FloatRect(0, -newPlayAreaHeight, newPlayAreaWidth, newPlayAreaHeight));
	tempLayerTexture.create(newPlayAreaWidth, newPlayAreaHeight);
	tempLayerTexture.setView(view);

	spriteHorizontalScale = view.getSize().x / tempLayerTexture.getSize().x;
	spriteVerticalScale = view.getSize().y / tempLayerTexture.getSize().y;
}

void DebugRenderSystem::setBackground(sf::Texture background) {
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
