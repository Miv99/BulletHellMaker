#pragma once
#include <Editor/CustomWidgets/ListView.h>
#include <Editor/CustomWidgets/EditBox.h>
#include <Editor/CopyPaste.h>
#include <Editor/Windows/EditorWindowConfirmationPromptChoice.h>
#include <Editor/EventCapturable.h>
#include <DataStructs/UndoStack.h>

class MainEditorWindow;

/*
Empty panel that captures undo/redo/copy/paste commands and whose purpose is for
AttackEditorPanel to show the properties of an EditorAttack.
Should be used only by AttackEditorPanel.

Signals:
AttackModified - emitted when the EditorAttack being edited is modified.
*/
class AttackEditorPropertiesPanel : public tgui::ScrollablePanel, public EventCapturable, public CopyPasteable {
public:
	tgui::Signal onAttackModify = { "AttackModified" };

	/*
	mainEditorWindow - the parent MainEditorWindow
	clipboard - the parent Clipboard
	attack - the EditorAttack being edited
	undoStackSize - the max size of this widget's undo stack
	*/
	AttackEditorPropertiesPanel(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, std::shared_ptr<EditorAttack> attack, int undoStackSize = 50);
	static std::shared_ptr<AttackEditorPropertiesPanel> create(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, std::shared_ptr<EditorAttack> attack, int undoStackSize = 50) {
		return std::make_shared<AttackEditorPropertiesPanel>(mainEditorWindow, clipboard, attack, undoStackSize);
	}

	CopyOperationResult copyFrom() override;
	PasteOperationResult pasteInto(std::shared_ptr<CopiedObject> pastedObject) override;
	PasteOperationResult paste2Into(std::shared_ptr<CopiedObject> pastedObject) override;

	bool handleEvent(sf::Event event) override;

	tgui::Signal& getSignal(tgui::String signalName) override;

	void manualPaste();

	std::shared_ptr<ListView> getUsedByListView();

private:
	MainEditorWindow& mainEditorWindow;
	Clipboard& clipboard;
	std::shared_ptr<EditorAttack> attack;
	UndoStack undoStack;

	// Lists the EditorAttackPatterns that use the EditorAttack being edited
	std::shared_ptr<ListView> usedBy;

	std::shared_ptr<EditBox> name;

	bool ignoreSignals = false;

	void manualUndo();
	void manualRedo();

	/*
	Called when the user responds to a prompt confirming an EditorAttack being pasted to overwrite the properties of the current EditorAttack being edited.
	*/
	void onPasteIntoConfirmation(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE choice, std::string newName);
};