#include "EditorMovablePointPanel.h"

EditorMovablePointPanel::EditorMovablePointPanel(EditorWindow & parentWindow, LevelPack & levelPack, std::shared_ptr<EditorMovablePoint> emp) : parentWindow(parentWindow), levelPack(levelPack), emp(emp) {
	tabs = TabsWithPanel::create(parentWindow);
	tabs->setPosition(0, 0);
	tabs->setSize("100%", "100%");
	add(tabs);
}

bool EditorMovablePointPanel::handleEvent(sf::Event event) {
	return false;
}

tgui::Signal & EditorMovablePointPanel::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onEMPModify.getName())) {
		return onEMPModify;
	}
	return tgui::Panel::getSignal(signalName);
}
