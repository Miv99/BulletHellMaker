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
handleEvent() is called from this widget's container, EditorWindow::LevelPackObjectsListPanel.
*/
class LevelPackObjectsListView : public ListViewScrollablePanel, public EventCapturable, public CopyPasteable {
public:
	/*
	mainEditorWindow - the parent MainEditorWindow using this widget
	clipboard - the parent Clipboard
	*/
	LevelPackObjectsListView(std::string copyPasteableID, MainEditorWindow& mainEditorWindow, Clipboard& clipboard, int undoStackSize = 50);

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

protected:
	static const std::string SAVED_LEVEL_PACK_OBJECT_ITEM_FORMAT;
	static const std::string UNSAVED_LEVEL_PACK_OBJECT_ITEM_FORMAT;

	enum class SORT_OPTION {
		ID,
		NAME
	};

	MainEditorWindow& mainEditorWindow;
	LevelPack* levelPack;
	UndoStack undoStack;
	Clipboard& clipboard;

	// Maps an EditorAttack ID to its index
	std::map<int, int> levelPackObjectIDToListViewIndexMap;
	// Maps an index to the associated EditorAtack ID
	std::map<int, int> listViewIndexToLevelPackObjectIDMap;
	// The current attack sort option
	SORT_OPTION sortOption;

	virtual void addLevelPackObjectsToListView() = 0;

	virtual std::map<int, std::shared_ptr<LevelPackObject>>& getUnsavedLevelPackObjects() = 0;
	virtual void updateLevelPackObjectInLevelPack(std::shared_ptr<LevelPackObject> obj) = 0;
	virtual void deleteLevelPackObjectInLevelPack(int id) = 0;
	virtual std::shared_ptr<LevelPackObject> getLevelPackObjectFromLevelPack(int id) = 0;
	virtual std::set<int> getNextLevelPackObjectIDs(int count) = 0;
	virtual void openLevelPackObjectInMainEditorWindow(int id) = 0;
	virtual void reloadLevelPackObjectTabInMainEditorWindow(int id) = 0;

	virtual std::string getPasteIntoConfirmationPrompt() = 0;

	/*
	Called when the confirmation popup from doing paste2 is answered.
	*/
	virtual void onPasteIntoConfirmation(bool confirmed, std::vector<std::shared_ptr<LevelPackObject>> newObjects) = 0;

	/*
	Helper function for addLevelPackObjectsToListView().

	objID - ID of the object
	objName - name of the object
	objIsUnsaved - whether the object is unsaved in mainEditorWindow
	objUnsavedName - the unsaved name of the object; only used if objIsUnsaved is true
	*/
	void addLevelPackObjectToListView(int objID, std::string objName, bool objIsUnsaved, std::string objUnsavedName);
};

class AttacksListView : public LevelPackObjectsListView {
public:
	AttacksListView(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, int undoStackSize = 50);
	static std::shared_ptr<AttacksListView> create(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, int undoStackSize = 50) {
		return std::make_shared<AttacksListView>(mainEditorWindow, clipboard, undoStackSize);
	}

protected:
	void addLevelPackObjectsToListView() override;

	std::map<int, std::shared_ptr<LevelPackObject>>& getUnsavedLevelPackObjects() override;
	void updateLevelPackObjectInLevelPack(std::shared_ptr<LevelPackObject> obj) override;
	void deleteLevelPackObjectInLevelPack(int id) override;
	std::shared_ptr<LevelPackObject> getLevelPackObjectFromLevelPack(int id) override;
	std::set<int> getNextLevelPackObjectIDs(int count) override;
	void openLevelPackObjectInMainEditorWindow(int id) override;
	void reloadLevelPackObjectTabInMainEditorWindow(int id) override;
	std::string getPasteIntoConfirmationPrompt() override;

	void onPasteIntoConfirmation(bool confirmed, std::vector<std::shared_ptr<LevelPackObject>> newObjects) override;
};