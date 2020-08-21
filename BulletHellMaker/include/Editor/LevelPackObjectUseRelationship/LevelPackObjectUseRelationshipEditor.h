#pragma once
#include <TGUI/TGUI.hpp>

#include <Editor/CustomWidgets/EditorUtilities.h>
#include <LevelPack/LevelPack.h>
#include <Editor/EditorWindow.h>

/*
A ListViewScrollablePanel of EditorAttacks, for use by MainEditorWindow.
Items should not be added or removed from this with the exception of from reload().
handleEvent() is called from this widget's container, EditorWindow::LevelPackObjectsListPanel.
*/
class LevelPackObjectUseRelationshipListView : public ListViewScrollablePanel, public EventCapturable, public CopyPasteable {
public:
	/*
	mainEditorWindow - the parent MainEditorWindow using this widget
	clipboard - the parent Clipboard
	*/
	LevelPackObjectUseRelationshipListView(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, int undoStackSize = 50);
	static std::shared_ptr<LevelPackObjectUseRelationshipListView> create(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, int undoStackSize = 50) {
		return std::make_shared<LevelPackObjectUseRelationshipListView>(mainEditorWindow, clipboard, undoStackSize);
	}

	std::pair<std::shared_ptr<CopiedObject>, std::string> copyFrom() override;
	std::string pasteInto(std::shared_ptr<CopiedObject> pastedObject) override;
	std::string paste2Into(std::shared_ptr<CopiedObject> pastedObject) override;

	bool handleEvent(sf::Event event) override;

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
	Does the select all command on this widget.
	*/
	void manualSelectAll();

	UndoStack& getUndoStack();

protected:
	static const std::string SAVED_LEVEL_PACK_OBJECT_ITEM_FORMAT;
	static const std::string UNSAVED_LEVEL_PACK_OBJECT_ITEM_FORMAT;

	enum class SORT_OPTION {
		ID,
		NAME
	};

	MainEditorWindow& mainEditorWindow;
	UndoStack undoStack;
	Clipboard& clipboard;
};

/*
A widget used to edit how an object uses LevelPackObjects.
*/
class LevelPackObjectUseRelationshipEditor : public tgui::Group {
public:
	LevelPackObjectUseRelationshipEditor(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, LevelPack& levelPack, int undoStackSize = 50);
	static std::shared_ptr<LevelPackObjectUseRelationshipEditor> create(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, LevelPack& levelPack, int undoStackSize = 50) {
		return std::make_shared<LevelPackObjectUseRelationshipEditor>(mainEditorWindow, clipboard, levelPack, undoStackSize);
	}

private:
	MainEditorWindow& mainEditorWindow;
	Clipboard& clipboard;
	LevelPack& levelPack;

	// Panel to hold listView
	std::shared_ptr<tgui::Panel> listPanel;
	std::shared_ptr<LevelPackObjectUseRelationshipListView> listView;

	/*
	
	*/
	void onLevelPackObjectDeleted();
	/*
	Reloads the relationship editor to reflect changes made to the LevelPackObject user's relationships from
	outside this widget.
	*/
	void reload();
};