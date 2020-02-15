#include "EditorMovablePointPanel.h"

EditorMovablePointPanel::EditorMovablePointPanel(EditorWindow & parentWindow, LevelPack & levelPack, std::shared_ptr<EditorMovablePoint> emp) : parentWindow(parentWindow), levelPack(levelPack), emp(emp) {
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
