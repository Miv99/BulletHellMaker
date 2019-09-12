#pragma once
#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <utility>
#include "Components.h"

/*
Render system that uses no shaders and can draw outside the play area.
*/
class DebugRenderSystem {
public:
	DebugRenderSystem(entt::DefaultRegistry& registry, sf::RenderWindow& window, float playAreaWidth, float playAreaHeight);

	void update(float deltaTime);

	/*
	Sets the resolution of the game.
	This doesn't change the size of the window. It only affects the gameplay quality.
	Resolutions that are too small won't work. 1600x900, 1024x768, and anything higher will work.
	*/
	void setResolution(int newPlayAreaWidth, int newPlayAreaHeight);
	void setBackground(sf::Texture background);
	inline void setBackgroundScrollSpeedX(float backgroundScrollSpeedX) { this->backgroundScrollSpeedX = backgroundScrollSpeedX; }
	inline void setBackgroundScrollSpeedY(float backgroundScrollSpeedY) { this->backgroundScrollSpeedY = backgroundScrollSpeedY; }

private:
	entt::DefaultRegistry& registry;
	sf::RenderWindow& window;

	sf::CircleShape circleFormat;

	struct SubLayerComparator {
		int operator()(const SpriteComponent& a, const SpriteComponent& b) {
			return a.getSubLayer() < b.getSubLayer();
		}
	};

	// Pairs of layer and entities in that layer
	// Entites in the same layer are sorted by sublayer
	std::vector<std::pair<int, std::vector<std::reference_wrapper<SpriteComponent>>>> layers;

	// Temporary layer texture for background
	sf::RenderTexture tempLayerTexture;

	// Black magic needed to scale texture conversion into sprites correctly since views do not match the texture size
	// I actually don't know why this is needed only when using 2 or more global shaders on a texture
	float spriteHorizontalScale;
	float spriteVerticalScale;

	sf::Texture background;
	// The background as a sprite
	sf::Sprite backgroundSprite;
	float backgroundScrollSpeedX, backgroundScrollSpeedY;
	// Current position of top-left corner of the screen relative to the top-left corner of the background
	float backgroundX = 0, backgroundY = 0;
	float backgroundTextureSizeX, backgroundTextureSizeY;
};