#include <Editor/LevelPackObjectList/LevelPackObjectsListPanel.h>

#include <Editor/LevelPackObjectList/LevelPackObjectsListView.h>

LevelPackObjectsListPanel::LevelPackObjectsListPanel(LevelPackObjectsListView& childListView)
	: childListView(childListView) {
}

bool LevelPackObjectsListPanel::handleEvent(sf::Event event) {
	if (childListView.handleEvent(event)) {
		return true;
	}
	return false;
}