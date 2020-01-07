#include "EditorInstance.h"
#include <TGUI/TGUI.hpp>
#include <SFML/Graphics.hpp>
#include "GameInstance.h"

int main() {
	//GameInstance a("test pack");
	//a.loadLevel(0);
	//a.start();
	//EditorInstance a("test pack");

	// Declare and create a new render-window
	sf::RenderWindow window(sf::VideoMode(800, 600), "SFML window");

	tgui::Gui gui;
	auto p = SimpleEngineRenderer::create(window);
	p->setSize("50%", "50%");
	p->setPosition("25%", "25%");
	gui.add(p);
	gui.setTarget(window);

	p->loadLevelPack("test pack");
	p->loadLevel(0);
	p->unpause();

	// Limit the framerate to 60 frames per second (this step is optional)
	window.setFramerateLimit(60);
	// The main loop - ends as soon as the window is closed
	while (window.isOpen()) {
		// Event processing
		sf::Event event;
		while (window.pollEvent(event)) {
			// Request for closing the window
			if (event.type == sf::Event::Closed)
				window.close();
		}
		// Clear the whole window before rendering a new frame
		window.clear();
		gui.draw();
		// End the current frame and display its contents on screen
		window.display();
	}

	std::system("pause");
	return 0;
}