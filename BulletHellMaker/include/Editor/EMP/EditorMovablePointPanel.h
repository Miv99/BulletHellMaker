#pragma once
#include <TGUI/TGUI.hpp>

#include <LevelPack/LevelPack.h>
#include <LevelPack/EditorMovablePoint.h>
#include <LevelPack/EditorMovablePointAction.h>
#include <LevelPack/EditorMovablePointSpawnType.h>
#include <Editor/EventCapturable.h>
#include <Editor/Windows/MainEditorWindow.h>
#include <DataStructs/UndoStack.h>
#include <Editor/Util/ExtraSignals.h>
#include <Editor/CustomWidgets/EditBox.h>
#include <Editor/CustomWidgets/AnimatableChooser.h>
#include <Editor/CustomWidgets/SoundSettingsGroup.h>
#include <Editor/CustomWidgets/SingleMarkerPlacer.h>
#include <Editor/CustomWidgets/ChildWindow.h>
#include <Editor/CopyPaste.h>
#include <Editor/EMPA/EMPABasedMovementEditorPanel.h>
#include <Editor/CustomWidgets/SymbolTableEditor.h>

/*
Panel used to edit an EditorMovablePoint.

Signals:
	EMPModified - emitted when the EMP being edited is modified.
		Optional parameter: a shared_ptr to the newly modified EditorMovablePoint
*/
class EditorMovablePointPanel : public tgui::ScrollablePanel, public EventCapturable, public CopyPasteable, public ValueSymbolTablesChangePropagator {
public:
	/*
	mainEditorWindow - the parent MainEditorWindow this widget belongs to
	levelPack - the LevelPack that emp is in
	emp - the EditorMovablePoint being edited
	*/
	EditorMovablePointPanel(MainEditorWindow& mainEditorWindow, std::shared_ptr<LevelPack> levelPack, SpriteLoader& spriteLoader, Clipboard& clipboard, std::shared_ptr<EditorMovablePoint> emp, int undoStackSize = 50);
	~EditorMovablePointPanel();
	static std::shared_ptr<EditorMovablePointPanel> create(MainEditorWindow& mainEditorWindow, std::shared_ptr<LevelPack> levelPack, SpriteLoader& spriteLoader, Clipboard& clipboard, std::shared_ptr<EditorMovablePoint> emp, int undoStackSize = 50) {
		return std::make_shared<EditorMovablePointPanel>(mainEditorWindow, levelPack, spriteLoader, clipboard, emp, undoStackSize);
	}

	CopyOperationResult copyFrom() override;
	PasteOperationResult pasteInto(std::shared_ptr<CopiedObject> pastedObject) override;
	PasteOperationResult paste2Into(std::shared_ptr<CopiedObject> pastedObject) override;

	bool handleEvent(sf::Event event) override;

	tgui::Signal& getSignal(std::string signalName) override;

	void propagateChangesToChildren() override;
	ValueSymbolTable getLevelPackObjectSymbolTable() override;

private:
	static const std::string PROPERTIES_TAB_NAME;
	static const std::string MOVEMENT_TAB_NAME;

	MainEditorWindow& mainEditorWindow;
	std::shared_ptr<LevelPack> levelPack;
	Clipboard& clipboard;
	UndoStack undoStack;
	std::shared_ptr<EditorMovablePoint> emp;

	// Symbol table editor child window.
	// The window is added to the GUI directly and will be removed in this widget's destructor.
	std::shared_ptr<ChildWindow> symbolTableEditorWindow;
	// Symbol table editor
	std::shared_ptr<ValueSymbolTableEditor> symbolTableEditor;

	std::shared_ptr<EMPABasedMovementEditorPanel> movementEditorPanel;

	// Properties tab panel
	std::shared_ptr<tgui::ScrollablePanel> propertiesPanel;
	// Widgets in properties tab
	std::shared_ptr<tgui::Label> id;
	std::shared_ptr<tgui::Label> empiAnimatableLabel;
	std::shared_ptr<AnimatableChooser> empiAnimatable;
	std::shared_ptr<tgui::CheckBox> empiLoopAnimation;
	std::shared_ptr<tgui::Label> empiBaseSpriteLabel;
	std::shared_ptr<AnimatableChooser> empiBaseSprite;
	std::shared_ptr<tgui::CheckBox> isBullet;
	std::shared_ptr<tgui::Label> empiHitboxRadiusLabel;
	std::shared_ptr<EditBox> empiHitboxRadius;
	std::shared_ptr<tgui::Label> empiDespawnTimeLabel;
	std::shared_ptr<tgui::Label> empiSpawnTypeLabel;
	std::shared_ptr<tgui::ComboBox> empiSpawnType;
	std::shared_ptr<tgui::Label> empiSpawnTypeTimeLabel;
	std::shared_ptr<EditBox> empiSpawnTypeTime;
	std::shared_ptr<tgui::Label> empiSpawnTypeXLabel;
	std::shared_ptr<tgui::Label> empiSpawnTypeYLabel;
	std::shared_ptr<tgui::Button> empiSpawnLocationManualSet;
	std::shared_ptr<tgui::Label> empiShadowTrailLifespanLabel;
	std::shared_ptr<EditBox> empiShadowTrailLifespan;
	std::shared_ptr<tgui::Label> empiShadowTrailIntervalLabel;
	std::shared_ptr<EditBox> empiShadowTrailInterval;
	std::shared_ptr<tgui::Label> empiDamageLabel;
	std::shared_ptr<EditBox> empiDamage;
	std::shared_ptr<tgui::Label> empiOnCollisionActionLabel;
	std::shared_ptr<tgui::ComboBox> empiOnCollisionAction;
	std::shared_ptr<tgui::Label> empiPierceResetTimeLabel;
	std::shared_ptr<EditBox> empiPierceResetTime;
	std::shared_ptr<tgui::Label> empiBulletModelLabel;
	std::shared_ptr<tgui::CheckBox> empiInheritRadius;
	std::shared_ptr<tgui::CheckBox> empiInheritDespawnTime;
	std::shared_ptr<tgui::CheckBox> empiInheritShadowTrailInterval;
	std::shared_ptr<tgui::CheckBox> empiInheritShadowTrailLifespan;
	std::shared_ptr<tgui::CheckBox> empiInheritAnimatables;
	std::shared_ptr<tgui::CheckBox> empiInheritDamage;
	std::shared_ptr<tgui::CheckBox> empiInheritPierceResetTime;
	std::shared_ptr<tgui::CheckBox> empiInheritSoundSettings;
	std::shared_ptr<EditBox> empiSpawnTypeX;
	std::shared_ptr<EditBox> empiSpawnTypeY;
	std::shared_ptr<SliderWithEditBox> empiDespawnTime;
	std::shared_ptr<tgui::Label> empiSoundSettingsLabel;
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

	std::shared_ptr<TabsWithPanel> tabs;

	/*
	Signal emitted when the EMP being edited is modified.
	Optional parameter: a shared_ptr to the newly modified EditorMovablePoint
	*/
	tgui::SignalEditorMovablePoint onEMPModify = { "EMPModified" };

	bool ignoreSignals = false;

	/*
	Update all widget values to match emp.
	*/
	void updateAllWidgetValues();

	void onLevelPackChange(LevelPack::LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE type, int id);
	void finishEditingSpawnTypePosition();

	/*
	Called when the user responds to a prompt confirming an EMP being pasted to overwrite the properties of the current EMP being edited.
	*/
	void onPasteIntoConfirmation(bool confirmed, std::shared_ptr<EditorMovablePoint> newEMP);

	/*
	Returns the unique ID of a BULLET_ON_COLLISION_ACTION.
	*/
	std::string getID(BULLET_ON_COLLISION_ACTION onCollisionAction);
	/*
	Returns the BULLET_ON_COLLISION_ACTION corresponding to some unique ID.
	*/
	BULLET_ON_COLLISION_ACTION fromID(std::string id);
	/*
	Returns the unique ID of an EMPSpawnType.
	*/
	std::string getID(std::shared_ptr<EMPSpawnType> spawnType);
};