#pragma once
#include "EditorWindow.h"
#include "Attack.h"
#include "EditorMovablePoint.h"

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
class AttackEditorWindow : public EditorWindow {
public:
	AttackEditorWindow(int width, int height);

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
	// nullptr if none selected
	std::shared_ptr<EditorAttack> selectedAttack;
	// nullptr if none selected
	std::shared_ptr<EditorMovablePoint> selectedEMP;
};