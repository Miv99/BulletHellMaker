#pragma once
#include "LevelPack.h"
#include "Attack.h"
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
AttackPatternBeginEdit - emitted when an EditorAttackPattern in the list of attack users is to be edited.
	Optional parameter: the ID of the EditorAttackPattern
AttackModified - emitted when the EditorAttack being edited is modified.
	Optional parameter: a shared_ptr to the newly modified EditorAttack
*/
class AttackEditorPanel : public tgui::Panel, public EventCapturable, public ValueSymbolTablesChangePropagator {
public:
	/*
	mainEditorWindow - the EditorWindow this widget is in
	levelPack - the LevelPack that attack belongs to
	clipboard - the parent Clipboard
	attack - the EditorAttack that is being edited
	undoStackSize - the maximum number of undos stored
	*/
	AttackEditorPanel(MainEditorWindow& mainEditorWindow, std::shared_ptr<LevelPack> levelPack, SpriteLoader& spriteLoader, Clipboard& clipboard, std::shared_ptr<EditorAttack> attack, int undoStackSize = 50);
	~AttackEditorPanel();
	static std::shared_ptr<AttackEditorPanel> create(MainEditorWindow& mainEditorWindow, std::shared_ptr<LevelPack> levelPack, SpriteLoader& spriteLoader, Clipboard& clipboard, std::shared_ptr<EditorAttack> attack, int undoStackSize = 50) {
		return std::make_shared<AttackEditorPanel>(mainEditorWindow, levelPack, spriteLoader, clipboard, attack, undoStackSize);
	}

	/*
	Returns the ID of an EMP, extracted from the string returned by getEMPTextInTreeView().

	text - the string returned by getEMPTextInTreeView()
	*/
	static int getEMPIDFromTreeViewText(std::string text);

	bool handleEvent(sf::Event event) override;

	tgui::Signal& getSignal(std::string signalName) override;

protected:
	void propagateChangesToChildren() override;

	ValueSymbolTable getLevelPackObjectSymbolTable() override;

private:
	static const std::string PROPERTIES_TAB_NAME;
	static const std::string EMP_TAB_NAME_FORMAT;

	MainEditorWindow& mainEditorWindow;
	std::shared_ptr<LevelPack> levelPack;
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

	// Symbol table editor child window.
	// The window is added to the GUI directly and will be removed in this widget's destructor.
	std::shared_ptr<tgui::ChildWindow> symbolTableEditorWindow;
	// Symbol table editor
	std::shared_ptr<ValueSymbolTableEditor> symbolTableEditor;

	std::shared_ptr<TabsWithPanel> tabs;
	// The properties tab
	std::shared_ptr<AttackEditorPropertiesPanel> propertiesPanel;
	// The ID of the EditorAttackPattern in usedBy that was just right clicked
	int usedByRightClickedAttackPatternID;
	// Maps an index in usedBy to the ID of the EditorAttackPattern being shown in that index
	std::map<int, int> usedByIDMap;

	// A TreeView of all the EMPs of the EditorAttack being edited
	std::shared_ptr<tgui::TreeView> empsTreeView;

	/*
	Opens a tab for editing an EMP of the attack being edited.

	empHierarchy - the hierarchy in empsTreeView to the node that represents the EMP to be edited
	*/
	void openEMPTab(std::vector<sf::String> empHierarchy);
	/*
	Opens a tab for editing an EMP of the attack being edited.

	empID - the ID of the EMP
	*/
	void openEMPTab(int empID);

	/*
	Called whenever onChange from levelPack is emitted.
	*/
	void onLevelPackChange(LevelPack::LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE type, int id);
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
	Reloads an EMP tab to reflect new changes to the associated EMP that didn't come from
	the tab itself. If the EMP no longer exists, the tab will be removed.
	*/
	void reloadEMPTab(int empID);

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
EMPModified - emitted when the an EMP belonging to the EditorAttack being edited is modified by this widget.
	Optional parameter - a shared_ptr to the modified EMP. This should only be used to reload EMP tabs, so this 
		will be nullptr if the tab doesn't need to be reloaded.
MainEMPChildDeleted - emitted when some child (but not necessarily direct child) EMP of the mainEMP of the EditorAttack
	being edited is deleted. The EMPModified signal will be emitted right before this one is.
	Optional parameter: the ID of the EMP that was deleted.
*/
class EditorMovablePointTreePanel : public tgui::Panel, public EventCapturable, public CopyPasteable {
public:
	/*
	parentAttackEditorPanel - the AttackEditorPanel this widget is a child of
	clipboard - the parent Clipboard
	attack - the EditorAttack whose EMP tree is being viewed
	*/
	EditorMovablePointTreePanel(AttackEditorPanel& parentAttackEditorPanel, Clipboard& clipboard, std::shared_ptr<EditorAttack> attack, int undoStackSize = 50);
	static std::shared_ptr<EditorMovablePointTreePanel> create(AttackEditorPanel& parentAttackEditorPanel, Clipboard& clipboard, std::shared_ptr<EditorAttack> attack, int undoStackSize = 50) {
		return std::make_shared<EditorMovablePointTreePanel>(parentAttackEditorPanel, clipboard, attack, undoStackSize);
	}

	std::pair<std::shared_ptr<CopiedObject>, std::string> copyFrom() override;
	std::string pasteInto(std::shared_ptr<CopiedObject> pastedObject) override;
	std::string paste2Into(std::shared_ptr<CopiedObject> pastedObject) override;

	bool handleEvent(sf::Event event) override;

	void manualCopy();
	void manualPaste();
	void manualPaste2();
	void manualDelete();

	tgui::Signal& getSignal(std::string signalName) override;
	std::shared_ptr<tgui::TreeView> getEmpsTreeView();

private:
	AttackEditorPanel& parentAttackEditorPanel;
	UndoStack undoStack;
	Clipboard& clipboard;
	std::shared_ptr<EditorAttack> attack;
	std::shared_ptr<tgui::TreeView> empsTreeView;

	tgui::SignalEditorMovablePoint onEMPModify = { "EMPModified" };
	tgui::SignalInt onMainEMPChildDeletion = { "MainEMPChildDeleted" };

	void manualUndo();
	void manualRedo();
};