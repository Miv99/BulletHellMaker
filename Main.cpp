#include <SFML/Graphics.hpp>
#include <iostream>
#include <entt/entt.hpp>
#include "Components.h"
#include "SpriteLoader.h"
#include "TextFileParser.h"
#include <boost/chrono.hpp>
#include "CollisionSystem.h"
#include "MovementSystem.h"
#include "RenderSystem.h"
#include "GameInstance.h"
// testing
#include "TGUI/TGUI.hpp"
#include "TextMarshallable.h"

int main() {
	sf::RenderWindow window(sf::VideoMode(1024, 768), "SFML works!");
	sf::Clock deltaClock;

	// testing
	GameInstance game(window, "test pack");
	game.startLevel(0);

	// Game loop
	// Render at 60fps
	float renderUpdateInterval = 1.0f / 60;
	while (window.isOpen()) {
		// While behind in render updates, do physics updates
		float timeSinceLastRender = 0;
		while (timeSinceLastRender < renderUpdateInterval) {
			sf::Event event;
			while (window.pollEvent(event)) {
				if (event.type == sf::Event::Closed)
					window.close();
			}

			float dt = deltaClock.restart().asSeconds();
			//float dt = 1 / 120.0f;
			game.physicsUpdate(dt);

			timeSinceLastRender += dt;
		}

		window.clear();

		float interpolation = timeSinceLastRender - renderUpdateInterval;
		game.render(timeSinceLastRender);
		//std::cout << 1.0f / timeSinceLastRender << std::endl;

		window.display();
	}

	std::system("pause");
	return 0;
}