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
AttackPatternDoubleClicked - emitted when an EditorAttackPattern in the list of attack users is double clicked.
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
	LevelPack& levelPack;
	UndoStack undoStack;

	/*
	Signal emitted when an EditorAttackPattern in the list of attack users is double clicked.
	Optional parameter: the ID of the EditorAttackPattern
	*/
	tgui::SignalInt onAttackPatternDoubleClick = { "AttackPatternDoubleClicked" };
	/*
	Signal emitted when the EditorAttack being edited is modified.
	Optional parameter: a shared_ptr to the newly modified EditorAttack
	*/
	tgui::SignalEditorAttack onAttackModify = { "AttackModified" };

	std::shared_ptr<EditorAttack> attack;

	std::shared_ptr<TabsWithPanel> tabs;
	// Lists the EditorAttackPatterns that use the EditorAttack being edited.
	std::shared_ptr<tgui::ListView> usedBy;
	// Maps an index in usedBy to the ID of the EditorAttackPattern being shown in that index
	std::map<int, int> usedByIDMap;

	void populatePropertiesUsedByList();
};