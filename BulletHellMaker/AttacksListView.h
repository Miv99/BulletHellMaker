#pragma once
#include "EditorUtilities.h"
#include "CopyPaste.h"
#include "LevelPack.h"
#include "EventCapturable.h"
#include "UndoStack.h"

class MainEditorWindow;

/*
A ListViewScrollablePanel of EditorAttacks, for use by MainEditorWindow.
Items should not be added or removed from this with the exception of from reload().
handleEvent() is called from this widget's container, EditorWindow::AttacksListPanel.
*/
class AttacksListView : public ListViewScrollablePanel, public EventCapturable, public CopyPasteable {
public:
	/*
	mainEditorWindow - the parent MainEditorWindow using this widget
	clipboard - the parent Clipboard
	*/
	AttacksListView(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, int undoStackSize = 50);
	static std::shared_ptr<AttacksListView> create(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, int undoStackSize = 50) {
		return std::make_shared<AttacksListView>(mainEditorWindow, clipboard, undoStackSize);
	}

	std::shared_ptr<CopiedObject> copyFrom() override;
	void pasteInto(std::shared_ptr<CopiedObject> pastedObject) override;
	void paste2Into(std::shared_ptr<CopiedObject> pastedObject) override;

	bool handleEvent(sf::Event event) override;

	void setLevelPack(LevelPack* levelPack);

	/*
	Reload the list of EditorAttacks from the LevelPack.
	*/
	void reload();

	/*
	Does the copy command on this widget.
	*/
	void manualCopy();
	/*
	Does the paste command on this widget.
	*/
	void manualPaste();
	/*
	Does the paste2 command on this widget.
	*/
	void manualPaste2();
	/*
	Does the delete command on this widget.
	*/
	void manualDelete();
	/*
	Does the save command on this widget.
	*/
	void manualSave();
	/*
	Does the save all command on this widget.
	*/
	void manualSaveAll();
	/*
	Does the select all command on this widget.
	*/
	void manualSelectAll();

	/*
	Cycles between sorting ascending by ID and alphanumerically by name.
	*/
	void cycleSortOption();

	int getAttackIDFromIndex(int index);
	int getIndexFromAttackID(int attackID);
	UndoStack& getUndoStack();

private:
	static const std::string SAVED_ATTACK_ITEM_FORMAT;
	static const std::string UNSAVED_ATTACK_ITEM_FORMAT;

	enum SORT_OPTION {
		ID,
		NAME
	};

	MainEditorWindow& mainEditorWindow;
	LevelPack* levelPack;
	UndoStack undoStack;
	Clipboard& clipboard;

	// Maps an EditorAttack ID to its index
	std::map<int, int> attackIDToAttacksListViewIndexMap;
	// Maps an index to the associated EditorAtack ID
	std::map<int, int> attacksListViewIndexToAttackIDMap;
	// The current attack sort option
	SORT_OPTION sortOption;


	/*
	Called when the confirmation popup from doing paste2 is answered.
	*/
	void onPasteIntoConfirmation(bool confirmed, std::vector<std::shared_ptr<EditorAttack>> newAttacks);
};