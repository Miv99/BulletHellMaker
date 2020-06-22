#include <TGUI/TGUI.hpp>
#include <SFML/Graphics.hpp>
#include "GameInstance.h"
#include <iostream>
#include "EditorWindow.h"
#include "exprtk.hpp"

int main() {
	GameInstance a("test pack");
	a.loadLevel(0);
	a.start();
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



	
	/*auto mutex = std::make_shared<std::recursive_mutex>();
	auto p = MainEditorWindow::create(mutex, "title", 1024, 768);
	p->loadLevelPack("test pack");
	p->start();*/

	//std::string expression_str = "abs((2 * x)  - 1)";

	//float x = 1.1;

	//// Register x with the symbol_table
	//exprtk::symbol_table<float> symbol_table;
	//symbol_table.add_variable("x", x);

	//// Instantiate expression and register symbol_table
	//exprtk::expression<float> expression;
	//expression.register_symbol_table(symbol_table);

	//// Instantiate parser and compile the expression
	//exprtk::parser<float> parser;
	//parser.compile(expression_str, expression);

	//float result = 0.0;

	//// Evaluate and print result for when x = 1.1
	//result = expression.value();

	
	std::system("pause");
	return 0;
}