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
Empty panel that captures undo/redo/copy/paste commands and whose purpose is for
AttackPatternEditorPanel to show the properties of an EditorAttackPattern.
Should be used only by AttackPatternEditorPanel.

Signals:
AttackPatternModified - emitted when the EditorAttackPattern being edited is modified.
*/
class AttackPatternEditorPropertiesPanel : public tgui::ScrollablePanel, public EventCapturable, public CopyPasteable {
public:
	/*
	mainEditorWindow - the parent MainEditorWindow
	clipboard - the parent Clipboard
	attackPattern - the EditorAttackPattern being edited
	undoStackSize - the max size of this widget's undo stack
	*/
	AttackPatternEditorPropertiesPanel(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, std::shared_ptr<EditorAttackPattern> attackPattern, int undoStackSize = 50);
	static std::shared_ptr<AttackPatternEditorPropertiesPanel> create(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, std::shared_ptr<EditorAttackPattern> attackPattern, int undoStackSize = 50) {
		return std::make_shared<AttackPatternEditorPropertiesPanel>(mainEditorWindow, clipboard, attackPattern, undoStackSize);
	}

	std::pair<std::shared_ptr<CopiedObject>, std::string> copyFrom() override;
	std::string pasteInto(std::shared_ptr<CopiedObject> pastedObject) override;
	std::string paste2Into(std::shared_ptr<CopiedObject> pastedObject) override;

	bool handleEvent(sf::Event event) override;

	tgui::Signal& getSignal(std::string signalName) override;

	void manualPaste();

	std::shared_ptr<ListViewScrollablePanel> getUsedByPanel();

private:
	MainEditorWindow& mainEditorWindow;
	Clipboard& clipboard;
	std::shared_ptr<EditorAttackPattern> attackPattern;
	UndoStack undoStack;

	// Lists the EditorEnemyPhases (and possibly EditorPlayer) that use the EditorAttackPattern being edited
	std::shared_ptr<ListViewScrollablePanel> usedBy;

	std::shared_ptr<EditBox> name;

	tgui::Signal onAttackPatternModify = { "AttackPatternModified" };

	bool ignoreSignals = false;

	void manualUndo();
	void manualRedo();

	/*
	Called when the user responds to a prompt confirming an EditorAttackPattern being pasted to overwrite the properties of the current EditorAttackPattern being edited.
	*/
	void onPasteIntoConfirmation(bool confirmed, std::shared_ptr<EditorAttackPattern> newAttackPattern);
};

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