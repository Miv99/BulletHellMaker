#pragma once
#include "Components.h"
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

/*
Shaders are implemented here with the assumption that there will never be user-customizable shaders that can
be unique to a sprite.
*/
class RenderSystem {
public:
	RenderSystem(entt::DefaultRegistry& registry, sf::RenderWindow& window) : registry(registry), window(window) {}
	void update(float deltaTime);

private:
	entt::DefaultRegistry& registry;
	sf::RenderWindow& window;
};