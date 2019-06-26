#pragma once
#include "EditorWindow.h"
#include "Attack.h"
#include "EditorMovablePoint.h"
#include "LevelPack.h"
#include "EditorUtilities.h"
#include "UndoStack.h"
#include <thread>
#include <TGUI/TGUI.hpp>
#include <vector>
#include <map>
#include <mutex>

/*
The editor for editing EditorAttacks.

Notes:
-An EditorAttack's mainEMP is always spawned attached to its parent with no offset and will never have any EMPActions.
 It is invisible to the user. The user can set a sound to play on t=0 of attack execution, which will internally edit the mainEMP's sound settings.
 Its lifespan is automatically set to be the maximum lifespan out of all its children in its child EMP tree.

 PROBLEM: what if there are children that never despawn? there will be a build up of mainEMPs that do nothing.
 SOLUTION: find some way to make the mainEMP and only the mainEMP despawn when all its children are gone

 It is invisible to the user.
*/
class AttackEditor {
public:
	AttackEditor(LevelPack& levelPack, std::shared_ptr<SpriteLoader> spriteLoader);

	void start();
	void close();

	/*
	Should be called every time the main window is resized.
	*/
	void onMainWindowResize(int windowWidth, int windowHeight);
	/*
	Should be called every time the play area window is resized.
	*/
	void onPlayAreaWindowResize(int windowWidth, int windowHeight);

private:
	const float GUI_PADDING_X = 20;
	const float GUI_PADDING_Y = 10;
	const int UNDO_STACK_MAX = 100;

	/*
	id - id of the EditorAttack
	*/
	void selectAttack(int id);
	void deselectAttack();
	/*
	id - id of the EditorMovablePoint in the currently selected EditorAttack
	*/
	void selectEMP(int id);
	void deselectEMP();
	/*
	index - index of the selected action in the selected EMP's list of EMPActions
	*/
	void selectEMPA(int index);
	void deselectEMPA();

	
	/*
	Begin editing an EditorMovablePoint that is part of the currently open EditorAttack.
	*/
	void beginMovablePointEditing(std::shared_ptr<EditorMovablePoint> movablePoint);
	/*
	Finish editing an EditorMovablePoint.
	*/
	void endMovablePointEditing();

	void createAttack();
	void deleteAttack(std::shared_ptr<EditorAttack> attack);
	void saveAttack(std::shared_ptr<EditorAttack> attack);
	void discardAttackChanges(std::shared_ptr<EditorAttack> attack);

	/*
	Creates a new EMP as a child of parent.

	empOwner - the EditorAttack the parent is from
	*/
	void createEMP(std::shared_ptr<EditorAttack> empOwner, std::shared_ptr<EditorMovablePoint> parent);
	/*
	empOwner - the EditorAttack the emp is from
	*/
	void deleteEMP(std::shared_ptr<EditorAttack> empOwner, std::shared_ptr<EditorMovablePoint> emp);

	/*
	Should be called when a change is made to any EditorAttack.

	skipUndoCommandCreation - see EditorAttack::skipUndoCommandCreation
	attackWasModified - true if the attack widget values are being updated because of a change in the attack
	*/
	void setAttackWidgetValues(std::shared_ptr<EditorAttack> attackWithUnsavedChanges, bool skipUndoCommandCreation, bool attackWasModified);
	/*
	Should be called when a change is made to any EMP.

	parentAttack - the EditorAttack emp belongs to
	skipUndoCommandCreation - see EditorAttack::skipUndoCommandCreation
	fromInit - true if this function was called to initialize EMP widget values (basically when this function is called without
		changes made to emp)
	*/
	void setEMPWidgetValues(std::shared_ptr<EditorMovablePoint> emp, std::shared_ptr<EditorAttack> parentAttack, bool skipUndoCommandCreation, bool fromInit);

	void buildEMPTree();
	static sf::String getEMPTreeNodeText(const EditorMovablePoint& emp);
	// Clears empiActions and adds all EMPActions from the selected EMP to it
	void buildEMPIActions();
	// Clears alList and adds all EditorAttacks to it
	void buildAttackList(bool autoscrollToBottom);

	void onMainWindowRender(float deltaTime);

	LevelPack& levelPack;

	std::shared_ptr<EditorWindow> mainWindow;
	// Window to view the play area
	std::shared_ptr<EditorWindow> playAreaWindow;

	UndoStack mainWindowUndoStack;
	UndoStack playAreaWindowUndoStack;

	const float MAIN_WINDOW_WIDTH = 1024;
	const float MAIN_WINDOW_HEIGHT = 768;

	//------------------ Attack info widgets (ai__) ---------------------	
	std::shared_ptr<tgui::ScrollablePanel> aiPanel;

	std::shared_ptr<tgui::Label> aiID;
	std::shared_ptr<tgui::EditBox> aiName;
	std::shared_ptr<tgui::CheckBox> aiPlayAttackAnimation;
	//------------------ EMP info widgets (empi__) --------------------------------
	std::shared_ptr<tgui::ScrollablePanel> empiPanel;

	std::shared_ptr<tgui::Label> empiID;
	std::shared_ptr<tgui::Label> empiIsBulletLabel;
	std::shared_ptr<tgui::CheckBox> empiIsBullet;
	std::shared_ptr<tgui::Label> empiHitboxRadiusLabel;
	std::shared_ptr<SliderWithEditBox> empiHitboxRadius;
	std::shared_ptr<tgui::Label> empiDespawnTimeLabel;
	//TODO: max value is sum of all Actions times; make sure to change when actions is changed
	std::shared_ptr<SliderWithEditBox> empiDespawnTime;
	std::shared_ptr<tgui::Label> empiActionsLabel;
	// Entry ID is index in list of the EMP's actions
	std::shared_ptr<tgui::ListBox> empiActions;
	// if no EMPA is selected, this button adds a new EMPA at index 0; otherwise add at selectedEMPAIndex - 1
	std::shared_ptr<tgui::Button> empiActionsAddAbove;
	// if no EMPA is selected, this button adds a new EMPA at last index; otherwise add at selectedEMPAIndex + 1
	std::shared_ptr<tgui::Button> empiActionsAddBelow;
	std::shared_ptr<tgui::Button> empiActionsDelete;
	std::shared_ptr<tgui::Label> empiSpawnTypeLabel;
	// Entry ID is from getID()
	std::shared_ptr<tgui::ComboBox> empiSpawnType;
	std::shared_ptr<tgui::Label> empiSpawnTypeTimeLabel;
	std::shared_ptr<NumericalEditBoxWithLimits> empiSpawnTypeTime;
	std::shared_ptr<tgui::Label> empiSpawnTypeXLabel;
	std::shared_ptr<NumericalEditBoxWithLimits> empiSpawnTypeX;
	std::shared_ptr<tgui::Label> empiSpawnTypeYLabel;
	std::shared_ptr<NumericalEditBoxWithLimits> empiSpawnTypeY;
	std::shared_ptr<tgui::Button> empiSpawnLocationManualSet;
	std::shared_ptr<tgui::Label> empiShadowTrailLifespanLabel;
	std::shared_ptr<SliderWithEditBox> empiShadowTrailLifespan;
	std::shared_ptr<tgui::Label> empiShadowTrailIntervalLabel;
	std::shared_ptr<SliderWithEditBox> empiShadowTrailInterval;
	std::shared_ptr<tgui::Label> empiAnimatableLabel;
	std::shared_ptr<AnimatableChooser> empiAnimatable;
	std::shared_ptr<tgui::Label> empiLoopAnimationLabel;
	std::shared_ptr<tgui::CheckBox> empiLoopAnimation;
	std::shared_ptr<tgui::Label> empiBaseSpriteLabel;
	std::shared_ptr<AnimatableChooser> empiBaseSprite;
	std::shared_ptr<tgui::Label> empiDamageLabel;
	std::shared_ptr<NumericalEditBoxWithLimits> empiDamage;
	std::shared_ptr<tgui::Label> empiOnCollisionActionLabel;
	// Entry ID obtained from getID()
	std::shared_ptr<tgui::ComboBox> empiOnCollisionAction;
	std::shared_ptr<tgui::Label> empiPierceResetTimeLabel;
	std::shared_ptr<SliderWithEditBox> empiPierceResetTime;
	std::shared_ptr<SoundSettingsGroup> empiSoundSettings;
	std::shared_ptr<tgui::Label> empiBulletModelLabel;
	// Entry ID is bullet model ID
	std::shared_ptr<tgui::ComboBox> empiBulletModel;
	//TODO: disable if bullet model not chosen
	std::shared_ptr<tgui::Label> empiInheritRadiusLabel;
	std::shared_ptr<tgui::CheckBox> empiInheritRadius;
	std::shared_ptr<tgui::Label> empiInheritDespawnTimeLabel;
	std::shared_ptr<tgui::CheckBox> empiInheritDespawnTime;
	std::shared_ptr<tgui::Label> empiInheritShadowTrailIntervalLabel;
	std::shared_ptr<tgui::CheckBox> empiInheritShadowTrailInterval;
	std::shared_ptr<tgui::Label> empiInheritShadowTrailLifespanLabel;
	std::shared_ptr<tgui::CheckBox> empiInheritShadowTrailLifespan;
	std::shared_ptr<tgui::Label> empiInheritAnimatablesLabel;
	std::shared_ptr<tgui::CheckBox> empiInheritAnimatables;
	std::shared_ptr<tgui::Label> empiInheritDamageLabel;
	std::shared_ptr<tgui::CheckBox> empiInheritDamage;
	std::shared_ptr<tgui::Label> empiInheritSoundSettingsLabel;
	std::shared_ptr<tgui::CheckBox> empiInheritSoundSettings;

	void onEmpiHitboxRadiusChange(float value);
	void onEmpiDespawnTimeChange(float value);
	void onEmpiShadowTrailLifespanChange(float value);
	void onEmpiShadowTrailIntervalChange(float value);
	void onEmpiPierceResetTimeChange(float value);
	//------------------ Attack list widgets (al__) --------------------------------
	std::shared_ptr<tgui::ScrollablePanel> alPanel;

	std::shared_ptr<tgui::ListBox> alList;
	std::shared_ptr<tgui::Button> alSaveAll;
	std::shared_ptr<tgui::Button> alDiscardAll;
	std::shared_ptr<tgui::Button> alCreateAttack;
	std::shared_ptr<tgui::Button> alDeleteAttack;
	//------------------ EMP list widgets (empl__) --------------------------------
	std::shared_ptr<tgui::ScrollablePanel> emplPanel;

	std::shared_ptr<tgui::Label> emplLabel;
	std::shared_ptr<tgui::TreeView> emplTree;
	std::shared_ptr<tgui::Button> emplCreateEMP;
	std::shared_ptr<tgui::Button> emplDeleteEMP;
	//------------------ Play area window widgets (paw__) --------------------------------
	
	//------------------ EMPA info widgets (empai__) (these are in the play area window)
	std::shared_ptr<tgui::ScrollablePanel> empaiPanel;

	//TODO: connect() for these
	std::shared_ptr<tgui::Label> empaiInfo;
	std::shared_ptr<tgui::Label> empaiDurationLabel;
	std::shared_ptr<SliderWithEditBox> empaiDuration;
	std::shared_ptr<tgui::Label> empaiPolarDistanceLabel;
	std::shared_ptr<TFVGroup> empaiPolarDistance; // only for MoveCustomPolarEMPA
	std::shared_ptr<tgui::Label> empaiPolarAngleLabel;
	std::shared_ptr<TFVGroup> empaiPolarAngle; // only for MoveCustomPolarEMPA
	std::shared_ptr<tgui::Label> empaiBezierControlPointsLabel;
	std::shared_ptr<tgui::ListBox> empaiBezierControlPoints; // only for MoveCustomBezierEMPA
	//TODO: button to add/delete bezier control points
	std::shared_ptr<tgui::Label> empaiAngleOffsetLabel;
	std::shared_ptr<EMPAAngleOffsetGroup> empaiAngleOffset; // for MoveCustomPolarEMPA and MoveCustomBezierEMPA
	std::shared_ptr<tgui::Label> empaiHomingStrengthLabel;
	std::shared_ptr<TFVGroup> empaiHomingStrength; // only for MovePlayerHomingEMPA
	std::shared_ptr<tgui::Label> empaiHomingSpeedLabel;
	std::shared_ptr<TFVGroup> empaiHomingSpeed; // only for MovePlayerHomingEMPA
	// -----------------------------------------------------------------------------------

	// Threads for the above windows
	std::thread mainWindowThread;
	std::thread playAreaWindowThread;

	// When this bool is true, any changes (to EditorAttacks, EMPs, EMPAs,...) will not create a command for the undo stack.
	// This is to prevent a redo from signalling a widget from
	// creating another undo command, since UndoStack already takes care of that.
	bool skipUndoCommandCreation = false;

	std::shared_ptr<SpriteLoader> spriteLoader;

	// nullptr if none selected
	std::shared_ptr<EditorAttack> selectedAttack;
	// nullptr if none selected
	std::shared_ptr<EditorMovablePoint> selectedEMP;
	// -1 if none selected; linked with selectedEMPA
	int selectedEMPAIndex = -1;
	// nullptr if none selected; linked with selectedEMPAIndex
	std::shared_ptr<EMPAction> selectedEMPA;

	// Maps EditorAttack ID to the EditorAttack copy, but with those changes.
	// If an entry does not exist for some ID, the attack with that ID has no unsaved changes.
	std::map<int, std::shared_ptr<EditorAttack>> unsavedAttacks;

	// Mutex used to make sure multiple tgui widgets aren't being instantiated at the same time in different threads.
	// tgui::Gui draw() calls also can't be done at the same time.
	// Apparently tgui gets super messed up with multithreading.
	std::shared_ptr<std::mutex> tguiMutex;
};