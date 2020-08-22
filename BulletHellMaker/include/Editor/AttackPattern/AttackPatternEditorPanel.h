#pragma once
#include <memory>

#include <TGUI/TGUI.hpp>

#include <LevelPack/LevelPack.h>
#include <LevelPack/AttackPattern.h>
#include <DataStructs/UndoStack.h>
#include <Editor/EMP/EditorMovablePointPanel.h>
#include <Editor/Util/ExtraSignals.h>
#include <Editor/EventCapturable.h>
#include <Editor/CustomWidgets/SymbolTableEditor.h>
#include <Editor/CustomWidgets/TabsWithPanel.h>
#include <Editor/AttackPattern/AttackPatternEditorPropertiesPanel.h>

class MainEditorWindow;

/*
Panel that contains everything needed to edit a specific EditorAttackPattern.

Signals:
EnemyPhaseBeginEdit - emitted when an EditorEnemyPhase in the list of enemy phase users is to be edited.
	Optional parameter: the ID of the EditorEnemyPhase
PlayerBeginEdit - emitted when the EditorPlayer in the list of enemy phase users is to be edited.
AttackPatternModified - emitted when the EditorAttackPattern being edited is modified.
	Optional parameter: a shared_ptr to the newly modified EditorAttackPattern
*/
class AttackPatternEditorPanel : public tgui::Panel, public EventCapturable, public ValueSymbolTablesChangePropagator {
public:
	/*
	mainEditorWindow - the EditorWindow this widget is in
	levelPack - the LevelPack that attack belongs to
	clipboard - the parent Clipboard
	attackPattern - the EditorAttackPattern that is being edited
	undoStackSize - the maximum number of undos stored
	*/
	AttackPatternEditorPanel(MainEditorWindow& mainEditorWindow, std::shared_ptr<LevelPack> levelPack, SpriteLoader& spriteLoader, Clipboard& clipboard, std::shared_ptr<EditorAttackPattern> attackPattern, int undoStackSize = 50);
	~AttackPatternEditorPanel();
	static std::shared_ptr<AttackPatternEditorPanel> create(MainEditorWindow& mainEditorWindow, std::shared_ptr<LevelPack> levelPack, SpriteLoader& spriteLoader, Clipboard& clipboard, std::shared_ptr<EditorAttackPattern> attackPattern, int undoStackSize = 50) {
		return std::make_shared<AttackPatternEditorPanel>(mainEditorWindow, levelPack, spriteLoader, clipboard, attackPattern, undoStackSize);
	}

	tgui::Signal& getSignal(std::string signalName) override;

protected:
	void propagateChangesToChildren() override;

	ValueSymbolTable getLevelPackObjectSymbolTable() override;

private:
	static const std::string PROPERTIES_TAB_NAME;
	static const std::string MOVEMENT_TAB_NAME;

	static const int USED_BY_ID_MAP_PLAYER_RESERVED_ID;

	MainEditorWindow& mainEditorWindow;
	std::shared_ptr<LevelPack> levelPack;
	SpriteLoader& spriteLoader;
	Clipboard& clipboard;
	UndoStack undoStack;

	// The attack pattern being edited
	std::shared_ptr<EditorAttackPattern> attackPattern;

	std::shared_ptr<TabsWithPanel> tabs;
	// The properties tab
	std::shared_ptr<AttackPatternEditorPropertiesPanel> propertiesPanel;

	// The ID of the EditorEnemyPhase (or USED_BY_ID_MAP_PLAYER_RESERVED_ID) in usedBy that was just right clicked
	int usedByRightClickedID;
	// Maps an index in usedBy to the ID of the EditorEnemyPhase being shown in that index.
	// ID -1 is reserved for representing the LevelPack's EditorPlayer
	std::map<int, int> usedByIDMap;

	/*
	Signal emitted when an EditorEnemyPhase in the list of attack pattern users is to be edited.
	Optional parameter: the ID of the EditorEnemyPhase
	*/
	tgui::SignalInt onEnemyPhaseBeginEdit = { "EnemyPhaseBeginEdit" };
	/*
	Signal emitted when an EditorPlayer in the list of attack pattern users is to be edited.
	*/
	tgui::Signal onPlayerBeginEdit = { "PlayerBeginEdit" };
	/*
	Signal emitted when the EditorAttackPattern being edited is modified.
	Optional parameter: a shared_ptr to the newly modified EditorAttackPattern
	*/
	tgui::SignalEditorAttackPattern onAttackPatternModify = { "AttackPatternModified" };

	/*
	Called whenever onChange from levelPack is emitted.
	*/
	void onLevelPackChange(LevelPack::LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE type, int id);
	/*
	Clear and populate usedBy, the list of EditorPlayer and/or EditorEnemyPhases that use the 
	EditorAttackPattern being edited.
	*/
	void populatePropertiesUsedByList();
};