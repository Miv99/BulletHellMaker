#pragma once
#include "LevelPack.h"
#include "AttackPattern.h"
#include "EventCapturable.h"
#include "UndoStack.h"
#include "EditorUtilities.h"
#include "EditorWindow.h"
#include "EditorMovablePointPanel.h"
#include "ExtraSignals.h"
#include "SymbolTableEditor.h"
#include <TGUI/TGUI.hpp>
#include <memory>

/*
Panel that contains everything needed to edit a specific EditorAttackPattern.

Signals:
EnemyPhaseBeginEdit - emitted when an EditorEnemyPhase in the list of enemy phase users is to be edited.
	Optional parameter: the ID of the EditorEnemyPhase
PlayerBeginEdit - emitted when the EditorPlayer in the list of enemy phase users is to be edited.
AttackPatternModified - emitted when the EditorAttackPattern being edited is modified.
	Optional parameter: a shared_ptr to the newly modified EditorAttackPattern
*/
class AttackPatternEditorPanel : public tgui::Panel, public EventCapturable, public ValueSymbolTablesChangePropagator {
public:
	/*
	mainEditorWindow - the EditorWindow this widget is in
	levelPack - the LevelPack that attack belongs to
	clipboard - the parent Clipboard
	attackPattern - the EditorAttackPattern that is being edited
	undoStackSize - the maximum number of undos stored
	*/
	AttackPatternEditorPanel(MainEditorWindow& mainEditorWindow, std::shared_ptr<LevelPack> levelPack, SpriteLoader& spriteLoader, Clipboard& clipboard, std::shared_ptr<EditorAttackPattern> attackPattern, int undoStackSize = 50);
	~AttackPatternEditorPanel();
	static std::shared_ptr<AttackPatternEditorPanel> create(MainEditorWindow& mainEditorWindow, std::shared_ptr<LevelPack> levelPack, SpriteLoader& spriteLoader, Clipboard& clipboard, std::shared_ptr<EditorAttackPattern> attackPattern, int undoStackSize = 50) {
		return std::make_shared<AttackPatternEditorPanel>(mainEditorWindow, levelPack, spriteLoader, clipboard, attackPattern, undoStackSize);
	}

	tgui::Signal& getSignal(std::string signalName) override;

protected:
	void propagateChangesToChildren() override;

	ValueSymbolTable getLevelPackObjectSymbolTable() override;

private:
	MainEditorWindow& mainEditorWindow;
	std::shared_ptr<LevelPack> levelPack;
	SpriteLoader& spriteLoader;
	Clipboard& clipboard;
	UndoStack undoStack;

	/*
	Signal emitted when an EditorEnemyPhase in the list of attack pattern users is to be edited.
	Optional parameter: the ID of the EditorEnemyPhase
	*/
	tgui::SignalInt onEnemyPhaseBeginEdit = { "EnemyPhaseBeginEdit" };
	/*
	Signal emitted when an EditorPlayer in the list of attack pattern users is to be edited.
	*/
	tgui::Signal onPlayerBeginEdit = { "PlayerBeginEdit" };
	/*
	Signal emitted when the EditorAttackPattern being edited is modified.
	Optional parameter: a shared_ptr to the newly modified EditorAttackPattern
	*/
	tgui::SignalEditorAttackPattern onAttackPatternModify = { "AttackPatternModified" };

	// The attack pattern being edited
	std::shared_ptr<EditorAttackPattern> attackPattern;
};