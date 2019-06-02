#include "EditorWindow.h"

EditorWindow::EditorWindow(std::string windowTitle, int width, int height) : windowTitle(windowTitle), windowWidth(width), windowHeight(height) {
}

void EditorWindow::start() {
	if (!window) {
		// SFML requires the RenderWindow to be created in the thread

		window = std::make_shared<sf::RenderWindow>(sf::VideoMode(windowWidth, windowHeight), windowTitle, sf::Style::Default);
		window->setKeyRepeatEnabled(false);
		window->setActive(true);

		// Centered at negative y because SFML has (0, 0) at the top-left, and RenderSystem negates y-values so that (0, 0) in every other aspect of this game is bottom-left.
		sf::View view(sf::Vector2f(MAP_WIDTH / 2.0f, -MAP_HEIGHT / 2.0f), sf::Vector2f(MAP_WIDTH, MAP_HEIGHT));
		window->setView(view);
	}

	sf::Clock deltaClock;

	// Main loop
	while (window->isOpen() && !windowCloseQueued) {
		// While behind in render updates, do physics updates
		float timeSinceLastRender = 0;
		while (timeSinceLastRender < RENDER_INTERVAL) {
			sf::Event event;
			while (window->pollEvent(event)) {
				if (event.type == sf::Event::Closed) {
					window->close();
				} else {
					handleEvent(event);
				}
			}

			if (paused) {
				break;
			}

			float dt = std::min(MAX_PHYSICS_DELTA_TIME, deltaClock.restart().asSeconds());
			physicsUpdate(dt);

			timeSinceLastRender += dt;
		}

		window->clear();
		render(timeSinceLastRender);
		window->display();
	}
}

void EditorWindow::pause() {
	paused = true;
}

void EditorWindow::resume() {
	paused = false;
	//playerSystem->onResume();
}

void EditorWindow::physicsUpdate(float deltaTime) {
}

void EditorWindow::render(float deltaTime) {
}

void EditorWindow::handleEvent(sf::Event event) {
}
