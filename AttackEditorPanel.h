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
Panel that contains everything needed to edit an EditorAttack.

Signals:
AttackPatternDoubleClicked - emitted when an EditorAttackPattern in the list of attack users is to be edited.
	Optional parameter: the ID of the EditorAttackPattern
AttackModified - emitted when the EditorAttack being edited is modified.
	Optional parameter: a shared_ptr to the newly modified EditorAttack
*/
class AttackEditorPanel : public tgui::Panel, public EventCapturable {
public:
	/*
	parentWindow - the EditorWindow this widget is in
	levelPack - the LevelPack that attack belongs to
	attack - the EditorAttack that is being edited
	undoStackSize - the maximum number of undos stored
	*/
	AttackEditorPanel(EditorWindow& parentWindow, LevelPack& levelPack, std::shared_ptr<EditorAttack> attack, int undoStackSize = 50);
	static std::shared_ptr<AttackEditorPanel> create(EditorWindow& parentWindow, LevelPack& levelPack, std::shared_ptr<EditorAttack> attack, int undoStackSize = 50) {
		return std::make_shared<AttackEditorPanel>(parentWindow, levelPack, attack, undoStackSize);
	}

	bool handleEvent(sf::Event event) override;

	tgui::Signal& getSignal(std::string signalName) override;

private:
	static const std::string PROPERTIES_TAB_NAME;
	static const std::string EMP_TAB_NAME_FORMAT;

	EditorWindow& parentWindow;
	LevelPack& levelPack;
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
	// Lists the EditorAttackPatterns that use the EditorAttack being edited.
	std::shared_ptr<ListViewScrollablePanel> usedBy;
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
	Returns the string to be shown for each EditorMovablePoint in empsTreeView.
	*/
	static sf::String getEMPTextInTreeView(const EditorMovablePoint& emp);
	/*
	Returns the ID of an EMP, extracted from the string returned by getEMPTextInTreeView().

	text - the string returned by getEMPTextInTreeView()
	*/
	static int getEMPIDFromTreeViewText(std::string text);
};