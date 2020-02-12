#include "AttackEditorPanel.h"

AttackEditorPanel::AttackEditorPanel(EditorWindow& parentWindow, std::shared_ptr<EditorAttack> attack, int undoStackSize) : undoStack(UndoStack(undoStackSize)), attack(attack) {
	tabs = TabsWithPanel::create(parentWindow);
	tabs->setPosition(0, 0);
	tabs->setSize("100%", "100%");
}

bool AttackEditorPanel::handleEvent(sf::Event event) {
	return false;
}
