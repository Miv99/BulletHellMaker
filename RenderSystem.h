#pragma once
#include "Components.h"
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <utility>
#include <vector>
#include <functional>

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
	std::map<int, sf::RenderTexture> layerTextures;
};