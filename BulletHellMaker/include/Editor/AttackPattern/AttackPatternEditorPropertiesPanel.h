#pragma once
#include <TGUI/TGUI.hpp>

#include <Editor/EventCapturable.h>
#include <Editor/CopyPaste.h>
#include <Editor/CustomWidgets/ListView.h>
#include <Editor/CustomWidgets/EditBox.h>
#include <Editor/LevelPackObjectUseRelationship/AttackPatternToAttackUseRelationshipEditor.h>
#include <Editor/Windows/EditorWindowConfirmationPromptChoice.h>
#include <DataStructs/UndoStack.h>

class MainEditorWindow;

struct CopiedAttackPatternProperties {
	CopiedAttackPatternProperties(std::string name, std::vector<std::tuple<std::string, int, ExprSymbolTable>> attacks) {
		this->name = name;
		this->attacks = attacks;
	}

	std::string name;
	std::vector<std::tuple<std::string, int, ExprSymbolTable>> attacks;
};

/*
Empty panel that captures undo/redo/copy/paste commands and whose purpose is for
AttackPatternEditorPanel to show the properties of an EditorAttackPattern.
Should be used only by AttackPatternEditorPanel.

Signals:
AttackPatternModified - emitted when the EditorAttackPattern being edited is modified.
*/
class AttackPatternEditorPropertiesPanel : public tgui::ScrollablePanel, public EventCapturable, public CopyPasteable {
public:
	tgui::Signal onAttackPatternModify = { "AttackPatternModified" };

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

	CopyOperationResult copyFrom() override;
	PasteOperationResult pasteInto(std::shared_ptr<CopiedObject> pastedObject) override;
	PasteOperationResult paste2Into(std::shared_ptr<CopiedObject> pastedObject) override;

	bool handleEvent(sf::Event event) override;

	tgui::Signal& getSignal(tgui::String signalName) override;

	std::shared_ptr<ListView> getUsedByListView();

	void setSymbolTablesHierarchy(std::vector<ValueSymbolTable> symbolTablesHierarchy);

private:
	MainEditorWindow& mainEditorWindow;
	Clipboard& clipboard;
	UndoStack undoStack;

	// The AttackPattern being edited
	std::shared_ptr<EditorAttackPattern> attackPattern;

	std::shared_ptr<AttackPatternToAttackUseRelationshipEditor> relationshipEditor;
	// Lists the EditorEnemyPhases (and possibly EditorPlayer) that use the EditorAttackPattern being edited
	std::shared_ptr<ListView> usedBy;

	std::shared_ptr<EditBox> name;

	bool ignoreSignals = false;

	void manualUndo();
	void manualRedo();

	/*
	Called when the user responds to a prompt confirming an EditorAttackPattern being pasted to overwrite the properties of the current EditorAttackPattern being edited.
	*/
	void onPasteIntoConfirmation(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE choice, CopiedAttackPatternProperties newProperties);
};