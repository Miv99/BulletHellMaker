#pragma once
#include "LevelPack.h"
#include "EditorMovablePoint.h"
#include "EditorMovablePointAction.h"
#include "EditorMovablePointSpawnType.h"
#include "EventCapturable.h"
#include "EditorWindow.h"
#include "UndoStack.h"
#include "ExtraSignals.h"
#include "EditorUtilities.h"
#include <TGUI/TGUI.hpp>

/*
Panel used to edit an EditorMovablePoint.

Signals:
	EMPModified - emitted when the EMP being edited is modified.
		Optional parameter: a shared_ptr to the newly modified EditorMovablePoint
*/
class EditorMovablePointPanel : public tgui::ScrollablePanel, public EventCapturable {
public:
	/*
	parentWindow - the top level EditorWindow this widget belongs to
	levelPack - the LevelPack that emp is in
	emp - the EditorMovablePoint being edited
	*/
	EditorMovablePointPanel(EditorWindow& parentWindow, LevelPack& levelPack, SpriteLoader& spriteLoader, std::shared_ptr<EditorMovablePoint> emp, int undoStackSize = 50);
	inline static std::shared_ptr<EditorMovablePointPanel> create(EditorWindow& parentWindow, LevelPack& levelPack, SpriteLoader& spriteLoader, std::shared_ptr<EditorMovablePoint> emp, int undoStackSize = 50) {
		return std::make_shared<EditorMovablePointPanel>(parentWindow, levelPack, spriteLoader, emp, undoStackSize);
	}

	bool handleEvent(sf::Event event) override;

	tgui::Signal& getSignal(std::string signalName) override;

private:
	static const std::string PROPERTIES_TAB_NAME;
	static const std::string EMPA_TAB_NAME_FORMAT;
	// The index in EMPA_TAB_NAME_FORMAT that marks the beginning of the action index.
	// This is used to extract the action index out of a tab name.
	static const int EMPA_TAB_NAME_FORMAT_NUMBER_INDEX;

	EditorWindow& parentWindow;
	LevelPack& levelPack;
	UndoStack undoStack;
	std::shared_ptr<EditorMovablePoint> emp;

	// Properties tab panel
	std::shared_ptr<tgui::ScrollablePanel> properties;
	// Some widgets in properties tab
	std::shared_ptr<NumericalEditBoxWithLimits> empiSpawnTypeX;
	std::shared_ptr<NumericalEditBoxWithLimits> empiSpawnTypeY;
	std::shared_ptr<SliderWithEditBox> empiDespawnTime;
	std::shared_ptr<SoundSettingsGroup> empiSoundSettings;
	std::shared_ptr<tgui::ComboBox> empiBulletModel;

	// For manually setting spawn type position
	bool placingSpawnLocation = false;
	std::shared_ptr<SingleMarkerPlacer> spawnTypePositionMarkerPlacer;
	std::shared_ptr<tgui::Button> spawnTypePositionMarkerPlacerFinishEditing;
	// Data from this panel before spawnLocationMarkerPlacerFinishEditing was clicked
	std::vector<std::shared_ptr<tgui::Widget>> savedWidgets;
	float horizontalScrollPos;
	float verticalScrollPos;

	// The index of the selected EMPA in the EMPA list widget. -1 if none selected.
	int selectedEMPAIndex = -1;

	std::shared_ptr<TabsWithPanel> tabs;

	/*
	Signal emitted when the EMP being edited is modified.
	Optional parameter: a shared_ptr to the newly modified EditorMovablePoint
	*/
	tgui::SignalEditorMovablePoint onEMPModify = { "EMPModified" };

	bool ignoreSignals = false;

	/*
	Clear and populate a ListBoxScrollablePanel with emp's actions.
	Should be called whenever any EMPA is changed.
	*/
	void populateEMPAList(std::shared_ptr<ListBoxScrollablePanel> actionsListBoxScrollablePanel);
	/*
	Create a panel for editing an EMPA.

	empa - the EMPA for which the panel will edit
	index - the index of empa in emp's actions list
	empiActions - the ListBoxScrollablePanel which will display all of emp's actions
	*/
	std::shared_ptr<tgui::Panel> createEMPAPanel(std::shared_ptr<EMPAction> empa, int index, std::shared_ptr<ListBoxScrollablePanel> empiActions);

	void onLevelPackChange();
	void finishEditingSpawnTypePosition();
};