#include "AttackPatternEditorPanel.h"

AttackPatternEditorPanel::AttackPatternEditorPanel(MainEditorWindow& mainEditorWindow, std::shared_ptr<LevelPack> levelPack, SpriteLoader& spriteLoader, Clipboard& clipboard, std::shared_ptr<EditorAttackPattern> attackPattern, int undoStackSize) 
	: mainEditorWindow(mainEditorWindow), levelPack(levelPack), spriteLoader(spriteLoader), clipboard(clipboard), undoStack(UndoStack(undoStackSize)), attackPattern(attackPattern) {
	// TODO
}

AttackPatternEditorPanel::~AttackPatternEditorPanel() {
	// TODO
	//levelPack->getOnChange()->sink().disconnect<AttackEditorPanel, &AttackEditorPanel::populatePropertiesUsedByList>(this);
	//mainEditorWindow.getGui()->remove(symbolTableEditorWindow);
}

tgui::Signal& AttackPatternEditorPanel::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onEnemyPhaseBeginEdit.getName())) {
		return onEnemyPhaseBeginEdit;
	} else if (signalName == tgui::toLower(onPlayerBeginEdit.getName())) {
		return onPlayerBeginEdit;
	} else if (signalName == tgui::toLower(onAttackPatternModify.getName())) {
		return onAttackPatternModify;
	}
	return tgui::Panel::getSignal(signalName);
}

void AttackPatternEditorPanel::propagateChangesToChildren() {
	// TODO
}

ValueSymbolTable AttackPatternEditorPanel::getLevelPackObjectSymbolTable() {
	return attackPattern->getSymbolTable();
}
