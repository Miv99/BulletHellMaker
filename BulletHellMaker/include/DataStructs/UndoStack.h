#pragma once
#include <deque>
#include <functional>

class UndoableCommand {
public:
	UndoableCommand(std::function<void()> command, std::function<void()> reverse);

	inline void execute() { command(); }
	inline void undo() { reverse(); }

private:
	std::function<void()> command;
	std::function<void()> reverse;
};

class UndoStack {
public:
	UndoStack(int maxCommands);

	void execute(UndoableCommand command);
	/*
	Returns true if an undo command was executed.
	*/
	bool undo();
	/*
	Returns true if a redo command was executed.
	*/
	bool redo();
	void clear();

private:
	// Maximum number of commands stored in the UndoStack
	int maxCommands;

	// New commands pushed to front
	std::deque<UndoableCommand> undoStack;
	std::deque<UndoableCommand> redoStack;
};