#include <SFML/Graphics.hpp>
#include <iostream>
#include <algorithm>
#include <entt/entt.hpp>
#include "Components.h"
#include "SpriteLoader.h"
#include "TextFileParser.h"
#include <boost/chrono.hpp>
#include "CollisionSystem.h"
#include "MovementSystem.h"
#include "RenderSystem.h"
#include "GameInstance.h"
#include "Constants.h"
// testing
#include "TGUI/TGUI.hpp"
#include "TextMarshallable.h"

int main() {
	sf::RenderWindow window(sf::VideoMode(1024, 768), "SFML works!");
	window.setKeyRepeatEnabled(false);
	sf::Clock deltaClock;

	// testing
	GameInstance game(window, "test pack");
	game.startLevel(0);

	// Game loop
	while (window.isOpen()) {
		// While behind in render updates, do physics updates
		float timeSinceLastRender = 0;
		while (timeSinceLastRender < RENDER_INTERVAL) {
			sf::Event event;
			while (window.pollEvent(event)) {
				if (event.type == sf::Event::Closed) {
					window.close();
				} else {
					game.handleEvent(event);
				}
			}

			float dt = std::min(MAX_PHYSICS_DELTA_TIME, deltaClock.restart().asSeconds());
			//std::cout << dt << std::endl;
			//float dt = 1 / 120.0f;
			game.physicsUpdate(dt);

			timeSinceLastRender += dt;
		}

		window.clear();
		game.render(timeSinceLastRender);
		window.display();
	}

	std::system("pause");
	return 0;
}