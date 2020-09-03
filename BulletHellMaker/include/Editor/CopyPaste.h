#pragma once
#include <string>
#include <memory>

#include <entt/entt.hpp>

#include <LevelPack/EditorMovablePoint.h>
#include <LevelPack/Attack.h>
#include <LevelPack/AttackPattern.h>
#include <LevelPack/Enemy.h>
#include <LevelPack/EnemyPhase.h>
#include <LevelPack/EditorMovablePointAction.h>

class CopiedObject {
public:
	CopiedObject(std::string copiedFromID);

	std::string getCopiedFromID();

private:
	// The ID of the CopyPasteable that this object was copied from
	std::string copiedFromID;
};

struct CopyOperationResult {
	CopyOperationResult(std::shared_ptr<CopiedObject> copiedObject, std::string description) {
		this->copiedObject = copiedObject;
		this->description = description;
	}

	std::shared_ptr<CopiedObject> copiedObject;
	std::string description;
};

struct PasteOperationResult {
	PasteOperationResult(bool success, std::string description) {
		this->success = success;
		this->description = description;
	}

	bool success;
	std::string description;
};

class CopyPasteable {
public:
	/*
	id - objects copied from CopyPasteSources with a particular ID can be pasted only to
		CopyPasteables with the same id
	*/
	CopyPasteable(std::string id);

	/*
	Returns the operation result.
	*/
	virtual CopyOperationResult copyFrom() = 0;
	/*
	Returns the operation result.
	*/
	virtual PasteOperationResult pasteInto(std::shared_ptr<CopiedObject> pastedObject) = 0;
	/*
	Same thing as pasteInto(), but for if there needs to be some alternate
	paste functionality.
	*/
	virtual PasteOperationResult paste2Into(std::shared_ptr<CopiedObject> pastedObject) = 0;

	std::string getID();

private:
	std::string id;
};

class Clipboard {
public:
	Clipboard();

	CopyOperationResult copy(std::shared_ptr<CopyPasteable> source);
	PasteOperationResult paste(std::shared_ptr<CopyPasteable> target);
	/*
	Alternate paste functionality.
	*/
	PasteOperationResult paste2(std::shared_ptr<CopyPasteable> target);

	CopyOperationResult copy(CopyPasteable* source);
	PasteOperationResult paste(CopyPasteable* target);
	PasteOperationResult paste2(CopyPasteable* target);

	void clear();

	std::shared_ptr<entt::SigH<void(std::string)>> getOnCopy();
	std::shared_ptr<entt::SigH<void(std::string)>> getOnPaste();

private:
	std::shared_ptr<CopiedObject> copied;

	// 1 parameter: the message to be displayed to the user explaining the copy operation result
	std::shared_ptr<entt::SigH<void(std::string)>> onCopy;
	// 1 parameter: the message to be displayed to the user explaining the paste/paste2 operation result
	std::shared_ptr<entt::SigH<void(std::string)>> onPaste;
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

class CopiedAttackPatternToAttackUseRelationship : public CopiedObject {
public:
	CopiedAttackPatternToAttackUseRelationship(std::string copiedFromID, std::vector<std::tuple<std::string, int, ExprSymbolTable>> relationships);

	std::vector<std::tuple<std::string, int, ExprSymbolTable>> getRelationships();

	/*
	Returns the number of copied relationships.
	*/
	int getRelationshipsCount();

private:
	std::vector<std::tuple<std::string, int, ExprSymbolTable>> relationships;
};