#pragma once
#include <string>
#include "EditorMovablePoint.h"
#include "Attack.h"
#include "AttackPattern.h"
#include "Enemy.h"
#include "EnemyPhase.h"
#include "EditorMovablePointAction.h"

class CopiedObject {
public:
	CopiedObject(std::string copiedFromID);

	std::string getCopiedFromID();

private:
	// The ID of the CopyPasteable that this object was copied from
	std::string copiedFromID;
};

class CopyPasteable {
public:
	/*
	id - objects copied from CopyPasteSources with a particular ID can be pasted only to
		CopyPasteables with the same id
	*/
	CopyPasteable(std::string id);

	virtual std::shared_ptr<CopiedObject> copyFrom() = 0;
	virtual void pasteInto(std::shared_ptr<CopiedObject> pastedObject) = 0;
	/*
	Same thing as pasteInto(), but for if there needs to be some alternate
	paste functionality.
	*/
	virtual void paste2Into(std::shared_ptr<CopiedObject> pastedObject) = 0;

	std::string getID();

private:
	std::string id;
};

class Clipboard {
public:

	void copy(std::shared_ptr<CopyPasteable> source);
	void paste(std::shared_ptr<CopyPasteable> target);
	/*
	Alternate paste functionality.
	*/
	void paste2(std::shared_ptr<CopyPasteable> target);

	void copy(CopyPasteable* source);
	void paste(CopyPasteable* target);
	void paste2(CopyPasteable* target);

	void clear();

private:
	std::shared_ptr<CopiedObject> copied;
};

class CopiedEditorMovablePoint : public CopiedObject {
public:
	/*
	emp - the EditorMovablePoint that will be deep-copied.
	*/
	CopiedEditorMovablePoint(std::string copiedFromID, std::shared_ptr<EditorMovablePoint> emp);

	/*
	Returns a deep copy of the stored EMP.
	*/
	std::shared_ptr<EditorMovablePoint> getEMP();

private:
	std::shared_ptr<EditorMovablePoint> emp;
};

class CopiedLevelPackObject : public CopiedObject {
public:
	/*
	objs - a list of the copied LevelPackObject. Every EditorAttack in here will be deep-copied.
	*/
	CopiedLevelPackObject(std::string copiedFromID, std::vector<std::shared_ptr<LevelPackObject>> objs);

	/*
	Returns a deep copy of the stored LevelPackObjects.
	*/
	std::vector<std::shared_ptr<LevelPackObject>> getLevelPackObjects();

	/*
	Returns the number of copied objects.
	*/
	int getLevelPackObjectsCount();

private:
	std::vector<std::shared_ptr<LevelPackObject>> objs;
};

class CopiedPiecewiseTFVSegment : public CopiedObject {
public:
	/*
	segment - the segment; the format is specified in PiecewiseTFV
	*/
	CopiedPiecewiseTFVSegment(std::string copiedFromID, std::pair<float, std::shared_ptr<TFV>> segment);

	/*
	Returns a deep copy of the stored segment.
	*/
	std::pair<float, std::shared_ptr<TFV>> getSegment();

private:
	std::pair<float, std::shared_ptr<TFV>> segment;
};

class CopiedEMPActions : public CopiedObject {
public:
	/*
	actions - a list of the copied EMPActions. Every EMPA in here will be deep-copied.
	*/
	CopiedEMPActions(std::string copiedFromID, std::vector<std::shared_ptr<EMPAction>> objs);

	/*
	Returns a deep copy of the stored EMPAs.
	*/
	std::vector<std::shared_ptr<EMPAction>> getActions();

	/*
	Returns the number of copied EMPAs.
	*/
	int getActionsCount();

private:
	std::vector<std::shared_ptr<EMPAction>> actions;
};

class CopiedMarker : public CopiedObject {
public:
	CopiedMarker(std::string copiedFromID, sf::CircleShape marker);

	sf::CircleShape getMarker();

private:
	sf::CircleShape marker;
};