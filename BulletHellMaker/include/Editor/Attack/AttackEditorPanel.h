#pragma once
#include <memory>

#include <TGUI/TGUI.hpp>

#include <DataStructs/UndoStack.h>
#include <Editor/EventCapturable.h>
#include <Editor/EMP/EditorMovablePointPanel.h>
#include <Editor/Util/ExtraSignals.h>
#include <Editor/CustomWidgets/SymbolTableEditor.h>
#include <Editor/CustomWidgets/TabsWithPanel.h>
#include <Editor/CustomWidgets/ChildWindow.h>
#include <Editor/Attack/AttackEditorPropertiesPanel.h>
#include <LevelPack/LevelPack.h>
#include <LevelPack/Attack.h>

class MainEditorWindow;

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
	Signal emitted when an EditorAttackPattern in the list of attack users is to be edited.
	Optional parameter: the ID of the EditorAttackPattern
	*/
	tgui::SignalInt onAttackPatternBeginEdit = { "AttackPatternBeginEdit" };
	/*
	Signal emitted when the EditorAttack being edited is modified.
	Optional parameter: a shared_ptr to the newly modified EditorAttack
	*/
	tgui::SignalEditorAttack onAttackModify = { "AttackModified" };

	/*
	mainEditorWindow - the EditorWindow this widget is in
	levelPack - the LevelPack that attack belongs to
	clipboard - the parent Clipboard
	attack - the EditorAttack that is being edited
	*/
	AttackEditorPanel(MainEditorWindow& mainEditorWindow, std::shared_ptr<LevelPack> levelPack, SpriteLoader& spriteLoader, Clipboard& clipboard, std::shared_ptr<EditorAttack> attack);
	~AttackEditorPanel();
	static std::shared_ptr<AttackEditorPanel> create(MainEditorWindow& mainEditorWindow, std::shared_ptr<LevelPack> levelPack, SpriteLoader& spriteLoader,
		Clipboard& clipboard, std::shared_ptr<EditorAttack> attack) {
		return std::make_shared<AttackEditorPanel>(mainEditorWindow, levelPack, spriteLoader, clipboard, attack);
	}

	/*
	Returns the ID of an EMP, extracted from the string returned by getEMPTextInTreeView().

	text - the string returned by getEMPTextInTreeView()
	*/
	static int getEMPIDFromTreeViewText(std::string text);

	bool handleEvent(sf::Event event) override;

	tgui::Signal& getSignal(tgui::String signalName) override;

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

	// The EditorAttack being edited
	std::shared_ptr<EditorAttack> attack;

	// Symbol table editor child window.
	// The window is added to the GUI directly and will be removed in this widget's destructor.
	std::shared_ptr<ChildWindow> symbolTableEditorWindow;
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
	void openEMPTab(std::vector<tgui::String> empHierarchy);
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
	Generates a list of string vectors such that, when each of the string vectors are added to a tgui::TreeView,
	the tree hierarchy of an EMP is created.

	nodeText - a function that takes an EMP and returns a string -- the text in the tgui::TreeView for the node for that EMP
	pathToThisEmp - the ordered series of strings from the root of the tree this EMP is part of, to this EMP.
		For example, if this EMP has id 3 and its tree looks like
			  0
			1   2
				  3
		then pathToThisEmp will be { nodeText(emp0), nodeText(emp2) }
	*/
	std::vector<std::vector<tgui::String>> generateTreeViewEmpHierarchy(const EditorMovablePoint& emp, 
		std::function<tgui::String(const EditorMovablePoint&)> nodeText, std::vector<tgui::String> pathToThisEmp);

	/*
	Returns the string to be shown for each EditorMovablePoint in empsTreeView.
	*/
	static tgui::String getEMPTextInTreeView(const EditorMovablePoint& emp);
};