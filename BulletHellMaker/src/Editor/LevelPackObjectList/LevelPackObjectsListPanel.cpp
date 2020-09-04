#include <Editor/LevelPackObjectList/LevelPackObjectsListPanel.h>

#include <Editor/LevelPackObjectList/LevelPackObjectsListView.h>

LevelPackObjectsListPanel::LevelPackObjectsListPanel(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, LevelPackObjectsListView& childListView)
	: mainEditorWindow(mainEditorWindow), clipboard(clipboard), childListView(childListView) {
	
}

bool LevelPackObjectsListPanel::handleEvent(sf::Event event) {
	if (childListView.handleEvent(event)) {
		return true;
	}
	return false;
}

void LevelPackObjectsListPanel::setLevelPack(LevelPack* levelPack) {
	this->levelPack = levelPack;
}