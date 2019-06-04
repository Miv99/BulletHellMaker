#pragma once
#include "EditorWindow.h"
#include "Attack.h"
#include "EditorMovablePoint.h"
#include "LevelPack.h"
#include <thread>
#include <TGUI/TGUI.hpp>
#include <map>
#include <mutex>

/*
A window for editing EditorAttacks.

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
	AttackEditor(LevelPack& levelPack);
	void start();
	void close();

	void selectAttack(int id);
	void deselectAttack();
	void deselectEMP();

	/*
	Creates a new EditorMovablePoint. 
	Its parent is determined by which EMP is currently selected.
	If no EMP is currently selected, the new EMP will be a child of the currently open EditorAttack's mainEMP.
	*/
	void createNewMovablePoint();

	/*
	Begin editing an EditorMovablePoint that is part of the currently open EditorAttack.
	*/
	void beginMovablePointEditing(std::shared_ptr<EditorMovablePoint> movablePoint);
	/*
	Finish editing an EditorMovablePoint.
	*/
	void endMovablePointEditing();

private:
	const float GUI_PADDING_X = 20;
	const float GUI_PADDING_Y = 10;

	/*
	Should be called when a change is made to any EditorAttack.
	*/
	void onAttackChange(std::shared_ptr<EditorAttack> attackWithUnsavedChanges);

	LevelPack& levelPack;

	// Window that shows info about the currently selected attack
	std::shared_ptr<EditorWindow> attackInfoWindow;
	// Window that shows info about the currently selected EMP
	std::shared_ptr<EditorWindow> empInfoWindow;
	// Window that shows a list of all EditorAttacks in the LevelPack
	std::shared_ptr<EditorWindow> attackListWindow;
	// Window that shows a tree view of all EMPs in the currently selected attack
	std::shared_ptr<EditorWindow> empListWindow;
	// Window to view the play area
	std::shared_ptr<EditorWindow> playAreaWindow;

	//------------------ Attack info window widgets (ai__) ---------------------	
	const float AI_WINDOW_WIDTH = 400;
	const float AI_WINDOW_HEIGHT = 200;
	
	std::shared_ptr<tgui::Label> aiId;
	std::shared_ptr<tgui::EditBox> aiName;
	std::shared_ptr<tgui::CheckBox> aiPlayAttackAnimation;
	//------------------ EMP info window widgets --------------------------------
	//------------------ Attack list window widgets --------------------------------
	std::shared_ptr<tgui::ListBox> alList;
	std::shared_ptr<tgui::Button> alSaveAll;
	std::shared_ptr<tgui::Button> alDiscardAll;
	//------------------ EMP list window widgets --------------------------------
	//------------------ Play area window widgets --------------------------------

	// Threads for the above windows
	std::thread attackInfoWindowThread;
	std::thread empInfoWindowThread;
	std::thread attackListWindowThread;
	std::thread empListWindowThread;
	std::thread playAreaWindowThread;

	// nullptr if none selected
	std::shared_ptr<EditorAttack> selectedAttack;
	// nullptr if none selected
	std::shared_ptr<EditorMovablePoint> selectedEMP;

	// Maps EditorAttack ID to the EditorAttack copy, but with those changes.
	// If an entry does not exist for some ID, the attack with that ID has no unsaved changes.
	std::map<int, std::shared_ptr<EditorAttack>> attackHasUnsavedChanges;

	// Mutex used to make sure multiple tgui widgets aren't being instantiated at the same time in different threads.
	// tgui::Gui draw() calls also can't be done at the same time.
	// Apparently tgui gets super messed up with multithreading.
	std::shared_ptr<std::mutex> tguiMutex;
};