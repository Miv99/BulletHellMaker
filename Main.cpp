#include <TGUI/TGUI.hpp>
#include <SFML/Graphics.hpp>
#include "GameInstance.h"
#include <iostream>
#include "EditorWindow.h"

int main() {
	//GameInstance a("test pack");
	//a.loadLevel(0);
	//a.start();
	//EditorInstance a("test pack");

	// Declare and create a new render-window
	sf::RenderWindow window(sf::VideoMode(800, 600), "SFML window");

	tgui::Gui gui;
	auto mutex = std::make_shared<std::recursive_mutex>();
	auto p = MainEditorWindow::create(mutex, "title", 1024, 768);
	p->loadLevelPack("test pack");
	p->start();

	std::system("pause");
	return 0;
}