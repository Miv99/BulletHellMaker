#pragma once
#include "LevelPack.h"
#include "Attack.h"
#include "EventCapturable.h"
#include "UndoStack.h"
#include "EditorUtilities.h"
#include "EditorWindow.h"
#include "EditorMovablePointPanel.h"
#include "ExtraSignals.h"
#include <TGUI/TGUI.hpp>
#include <memory>


/*
Empty panel that captures undo/redo/copy/paste commands and whose purpose is for
AttackEditorPanel to show the properties of an EditorAttack.
Should be used only by AttackEditorPanel.

Signals:
AttackModified - emitted when the EditorAttack being edited is modified.
*/
class AttackEditorPropertiesPanel : public tgui::ScrollablePanel, public EventCapturable, public CopyPasteable {
public:
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

	std::shared_ptr<CopiedObject> copyFrom() override;
	void pasteInto(std::shared_ptr<CopiedObject> pastedObject) override;
	void paste2Into(std::shared_ptr<CopiedObject> pastedObject) override;

	bool handleEvent(sf::Event event) override;

	tgui::Signal& getSignal(std::string signalName) override;

	void manualPaste();

	std::shared_ptr<ListViewScrollablePanel> getUsedByPanel();

private:
	MainEditorWindow& mainEditorWindow;
	Clipboard& clipboard;
	std::shared_ptr<EditorAttack> attack;
	UndoStack undoStack;

	// Lists the EditorAttackPatterns that use the EditorAttack being edited
	std::shared_ptr<ListViewScrollablePanel> usedBy;

	std::shared_ptr<EditBox> name;

	tgui::Signal onAttackModify = { "AttackModified" };

	bool ignoreSignals = false;

	void manualUndo();
	void manualRedo();

	/*
	Called when the user responds to a prompt confirming an EditorAttack being pasted to overwrite the properties of the current EditorAttack being edited.
	*/
	void onPasteIntoConfirmation(bool confirmed, std::shared_ptr<EditorAttack> newAttack);
};

/*
Panel that contains everything needed to edit a specific EditorAttack.

Signals:
AttackPatternDoubleClicked - emitted when an EditorAttackPattern in the list of attack users is to be edited.
	Optional parameter: the ID of the EditorAttackPattern
AttackModified - emitted when the EditorAttack being edited is modified.
	Optional parameter: a shared_ptr to the newly modified EditorAttack
*/
class AttackEditorPanel : public tgui::Panel, public EventCapturable {
public:
	/*
	mainEditorWindow - the EditorWindow this widget is in
	levelPack - the LevelPack that attack belongs to
	clipboard - the parent Clipboard
	attack - the EditorAttack that is being edited
	undoStackSize - the maximum number of undos stored
	*/
	AttackEditorPanel(MainEditorWindow& mainEditorWindow, LevelPack& levelPack, SpriteLoader& spriteLoader, Clipboard& clipboard, std::shared_ptr<EditorAttack> attack, int undoStackSize = 50);
	~AttackEditorPanel();
	static std::shared_ptr<AttackEditorPanel> create(MainEditorWindow& mainEditorWindow, LevelPack& levelPack, SpriteLoader& spriteLoader, Clipboard& clipboard, std::shared_ptr<EditorAttack> attack, int undoStackSize = 50) {
		return std::make_shared<AttackEditorPanel>(mainEditorWindow, levelPack, spriteLoader, clipboard, attack, undoStackSize);
	}

	/*
	Returns the ID of an EMP, extracted from the string returned by getEMPTextInTreeView().

	text - the string returned by getEMPTextInTreeView()
	*/
	static int getEMPIDFromTreeViewText(std::string text);

	bool handleEvent(sf::Event event) override;

	tgui::Signal& getSignal(std::string signalName) override;

private:
	static const std::string PROPERTIES_TAB_NAME;
	static const std::string EMP_TAB_NAME_FORMAT;

	MainEditorWindow& mainEditorWindow;
	LevelPack& levelPack;
	SpriteLoader& spriteLoader;
	Clipboard& clipboard;
	UndoStack undoStack;

	/*
	Signal emitted when an EditorAttackPattern in the list of attack users is to be edited.
	Optional parameter: the ID of the EditorAttackPattern
	*/
	tgui::SignalInt onAttackPatternBeginEdit = { "AttackPatternBeginEdit" };
	/*
	Signal emitted when the EditorAttack being edited is modified.
	Optional parameter: a shared_ptr to the newly modified EditorAttack
	*/
	tgui::SignalEditorAttack onAttackModify = { "AttackModified" };

	// The EditorAttack being edited
	std::shared_ptr<EditorAttack> attack;

	std::shared_ptr<TabsWithPanel> tabs;
	// The properties tab
	std::shared_ptr<AttackEditorPropertiesPanel> properties;
	// The ID of the EditorAttackPattern in usedBy that was just right clicked
	int usedByRightClickedAttackPatternID;
	// Maps an index in usedBy to the ID of the EditorAttackPattern being shown in that index
	std::map<int, int> usedByIDMap;

	// A TreeView of all the EMPs of the EditorAttack being edited
	std::shared_ptr<tgui::TreeView> empsTreeView;
	// The hierarchy of the node in empsTreeView that was just right clicked
	std::vector<sf::String> empRightClickedNodeHierarchy;

	/*
	Opens a tab for editing an EMP of the attack being edited.

	empHierarchy - the hierarchy in empsTreeView to the node that represents the EMP to be edited
	*/
	void openEMPTab(std::vector<sf::String> empHierarchy);

	/*
	Clear and populate usedBy, the list of EditorAttackPatterns that use the 
	EditorAttack being edited.
	*/
	void populatePropertiesUsedByList();
	/*
	Clear and populate empsTree.
	*/
	void populateEMPsTreeView();

	/*
	Does the save command on this widget.
	*/
	void manualSave();

	/*
	Returns the string to be shown for each EditorMovablePoint in empsTreeView.
	*/
	static sf::String getEMPTextInTreeView(const EditorMovablePoint& emp);
};

/*
Empty panel that captures undo/redo/copy/paste commands and whose purpose is for
AttackEditorPanel to show the EMP tree of an EditorAttack.
Should be used only by AttackEditorPanel.

Signals:
MainEMPModified - emitted when the mainEMP of the EditorAttack being edited is modified
	by this widget.
MainEMPChildDeleted - emitted when some child EMP of the mainEMP of the EditorAttack
	being edited is deleted. The MainEMPModified signal will be emitted right before
	this one is.
	Optional parameter: the ID of the EMP that was deleted.
*/
class EditorMovablePointTreePanel : public tgui::Panel, public EventCapturable, public CopyPasteable {
public:
	/*
	clipboard - the parent Clipboard
	attack - the EditorAttack whose EMP tree is being viewed
	*/
	EditorMovablePointTreePanel(Clipboard& clipboard, std::shared_ptr<EditorAttack> attack, int undoStackSize = 50);
	static std::shared_ptr<EditorMovablePointTreePanel> create(Clipboard& clipboard, std::shared_ptr<EditorAttack> attack, int undoStackSize = 50) {
		return std::make_shared<EditorMovablePointTreePanel>(clipboard, attack, undoStackSize);
	}

	std::shared_ptr<CopiedObject> copyFrom() override;
	void pasteInto(std::shared_ptr<CopiedObject> pastedObject) override;
	void paste2Into(std::shared_ptr<CopiedObject> pastedObject) override;

	bool handleEvent(sf::Event event) override;

	tgui::Signal& getSignal(std::string signalName) override;

	std::shared_ptr<tgui::TreeView> getEmpsTreeView();

private:
	UndoStack undoStack;
	Clipboard& clipboard;
	std::shared_ptr<EditorAttack> attack;
	std::shared_ptr<tgui::TreeView> empsTreeView;

	tgui::Signal onMainEMPModify = { "MainEMPModified" };
	tgui::SignalInt onMainEMPChildDeletion = { "MainEMPChildDeleted" };

	void manualDelete();
	void manualUndo();
	void manualRedo();
	void manualCopy();
	void manualPaste();
	void manualPaste2();
};