#pragma once
#include "Components.h"
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <utility>
#include <vector>
#include <functional>
#include <memory>

/*
Shaders are implemented here with the assumption that there will never be user-customizable shaders that can
be unique to a sprite.
*/
class RenderSystem {
public:
	/*
	Window's view should already be set and should not be changed.
	*/
	RenderSystem(entt::DefaultRegistry& registry, sf::RenderWindow& window);
	void update(float deltaTime);

private:
	entt::DefaultRegistry& registry;
	sf::RenderWindow& window;

	struct SubLayerComparator {
		int operator()(const SpriteComponent& a, const SpriteComponent& b) {
			return a.getSubLayer() < b.getSubLayer();
		}
	};

	// Pairs of layer and entities in that layer
	// Entites in the same layer are sorted by sublayer
	std::vector<std::pair<int, std::vector<std::reference_wrapper<SpriteComponent>>>> layers;
	// Maps layer to the texture, onto which all sprites in a layer on drawn
	// All textures are the same size
	std::map<int, sf::RenderTexture> layerTextures;
	// Maps layer to the global shaders applied on that layer
	std::map<int, std::vector<std::unique_ptr<sf::Shader>>> globalShaders;

	// Temporary layer texture for using multiple shaders
	sf::RenderTexture tempLayerTexture;

	// Black magic needed to scale texture conversion into sprites correctly since views do not match the texture size
	// I actually don't know why this is needed only when using 2 or more global shaders on a texture
	float spriteHorizontalScale;
	float spriteVerticalScale;
};