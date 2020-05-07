#pragma once
#include "EditorUtilities.h"
#include "CopyPaste.h"
#include "LevelPack.h"

class MainEditorWindow;

/*
A ListViewScrollablePanel of EditorAttacks, for use by MainEditorWindow.
Items should not be added or removed from this with the exception of from reload().
*/
class AttacksListView : public ListViewScrollablePanel, public CopyPasteable {
public:
	/*
	mainEditorWindow - the parent MainEditorWindow using this widget
	clipboard - the parent Clipboard
	unsavedAttacks - the unsavedAttacks field from mainEditorWindow
	*/
	AttacksListView(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, std::map<int, std::shared_ptr<EditorAttack>>& unsavedAttacks);
	static std::shared_ptr<AttacksListView> create(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, std::map<int, std::shared_ptr<EditorAttack>>& unsavedAttacks) {
		return std::make_shared<AttacksListView>(mainEditorWindow, clipboard, unsavedAttacks);
	}

	std::shared_ptr<CopiedObject> copyFrom() override;
	void pasteInto(std::shared_ptr<CopiedObject> pastedObject) override;
	void paste2Into(std::shared_ptr<CopiedObject> pastedObject) override;

	void setLevelPack(LevelPack* levelPack);

	/*
	Reload the list of EditorAttacks from the LevelPack.
	*/
	void reload();

	int getAttackIDFromIndex(int index);
	int getIndexFromAttackID(int attackID);

private:
	MainEditorWindow& mainEditorWindow;
	LevelPack* levelPack;
	Clipboard& clipboard;
	// From MainEditorWindow
	std::map<int, std::shared_ptr<EditorAttack>>& unsavedAttacks;

	// Maps an EditorAttack ID to its index
	std::map<int, int> attackIDToAttacksListViewIndexMap;
	// Maps an index to the associated EditorAtack ID
	std::map<int, int> attacksListViewIndexToAttackIDMap;
};

class CopiedEditorAttack : public CopiedObject {
public:
	/*
	attacks - a list of the copied EditorAttacks. Every EditorAttack in here will be deep-copied.
	*/
	CopiedEditorAttack(std::string copiedFromID, std::vector<std::shared_ptr<EditorAttack>> attacks);

	std::vector<std::shared_ptr<EditorAttack>> getAttacks();

private:
	std::vector<std::shared_ptr<EditorAttack>> attacks;
};