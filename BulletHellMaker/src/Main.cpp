#include <iostream>

#include <TGUI/TGUI.hpp>
#include <SFML/Graphics.hpp>

#include <Config.h>
#include <Util/Logger.h>
#include <Game/GameInstance.h>
#include <Editor/Windows/MainEditorWindow.h>
#include <exprtk.hpp>

int main() {
	initLogger(linfo);

	try {
		/*
		GameInstance a("test pack");
		a.loadLevel(0);
		a.start();
		*/

		auto p = MainEditorWindow::create("title", 1024, 768);
		p->loadLevelPack("test pack");
		p->start();
	} catch (const char* e) {
		// Unhandled exeception
		L_(lerror) << "Uncaught exception: " << e << std::endl;
	}

	endLogger();
	
	std::system("pause");
	return 0;
}