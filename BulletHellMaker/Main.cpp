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

	//std::string expression_str = "abs((2 * x)  - 1 + y)";

	//float x = 1.1;

	//// register x with the symbol_table
	//exprtk::symbol_table<float> symbol_table;
	//symbol_table.add_constant("x", x);

	//// instantiate expression and register symbol_table
	//exprtk::expression<float> expression;

	//exprtk::symbol_table<float> unknown_var_st;
	//expression.register_symbol_table(unknown_var_st);
	//expression.register_symbol_table(symbol_table);

	//// instantiate parser and compile the expression
	//exprtk::parser<float> parser;
	//parser.enable_unknown_symbol_resolver();
	//parser.compile(expression_str, expression);

	//std::vector<std::string> variableList;
	//unknown_var_st.get_variable_list(variableList);
	//// The expression is legal only if every symbol in unknownVarSymbolTable (meaning every symbol used in expressionStr)
	//// is also in any symbol_table in symbolTables
	//for (std::string& symbolName : variableList) {
	//	std::cout << symbolName << std::endl;
	//}

	//// evaluate and print result for when x = 1.1
	//float result = expression.value();

	
	std::system("pause");
	return 0;
}