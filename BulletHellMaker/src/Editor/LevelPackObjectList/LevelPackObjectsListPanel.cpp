#include <Editor/LevelPackObjectList/LevelPackObjectsListPanel.h>


LevelPackObjectsListPanel::LevelPackObjectsListPanel(MainEditorWindow& mainEditorWindow, Clipboard& clipboard) : mainEditorWindow(mainEditorWindow), clipboard(clipboard) {
	
}

bool LevelPackObjectsListPanel::handleEvent(sf::Event event) {
	if (mainEditorWindow.getAttacksListView()->handleEvent(event)) {
		return true;
	}
	return false;
}

void LevelPackObjectsListPanel::setLevelPack(LevelPack* levelPack) {
	this->levelPack = levelPack;
}