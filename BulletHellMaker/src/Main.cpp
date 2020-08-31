#include <iostream>

#include <TGUI/TGUI.hpp>
#include <SFML/Graphics.hpp>

#include <Game/GameInstance.h>
#include <Editor/EditorWindow.h>
#include <exprtk.hpp>

int main() {
	/*GameInstance a("test pack");
	a.loadLevel(0);
	a.start();*/
	//EditorInstance a("test pack");

	// Declare and create a new render-window
	//sf::RenderWindow window(sf::VideoMode(800, 600), "SFML window");
	//tgui::Gui gui;
	//gui.setTarget(window);

	//auto t = MarkerPlacer::create(window);
	//t->insertMarker(0, sf::Vector2f(100, 100), sf::Color::Red);
	//t->insertMarker(0, sf::Vector2f(200, 200), sf::Color::Blue);
	//gui.add(t);
	//t->lookAt(sf::Vector2f(200, 200));

	//while (window.isOpen()) {
	//	sf::Event event;
	//	while (window.pollEvent(event)) {
	//		// When the window is closed, the application ends
	//		if (event.type == sf::Event::Closed)
	//			window.close();

	//		// Pass the event to all the widgets
	//		gui.handleEvent(event);
	//		t->handleEvent(event);
	//	}

	//	window.clear();

	//	// Draw all created widgets
	//	gui.draw();

	//	window.display();
	//}



	
	auto p = MainEditorWindow::create("title", 1024, 768);
	p->loadLevelPack("test pack");
	p->start();

	
	std::system("pause");
	return 0;
}