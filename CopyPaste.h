#pragma once
#include <string>
#include "EditorMovablePoint.h"
#include "Attack.h"

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

class CopiedEditorAttack : public CopiedObject {
public:
	/*
	attacks - a list of the copied EditorAttacks. Every EditorAttack in here will be deep-copied.
	*/
	CopiedEditorAttack(std::string copiedFromID, std::vector<std::shared_ptr<EditorAttack>> attacks);

	/*
	Returns a deep copy of the stored EditorAttacks.
	*/
	std::vector<std::shared_ptr<EditorAttack>> getAttacks();

private:
	std::vector<std::shared_ptr<EditorAttack>> attacks;
};