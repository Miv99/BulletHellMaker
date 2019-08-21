#pragma once
#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>

class DebugRenderSystem {
public:
	DebugRenderSystem(entt::DefaultRegistry& registry, sf::RenderWindow& window);

	void update(float deltaTime);

private:
	entt::DefaultRegistry& registry;
	sf::RenderWindow& window;

	sf::CircleShape circleFormat;
};