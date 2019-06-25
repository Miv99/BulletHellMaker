#pragma once
#include <deque>
#include <functional>

class UndoableCommand {
public:
	inline UndoableCommand(std::function<void()> command, std::function<void()> reverse) : command(command), reverse(reverse) {}

	inline void execute() { command(); }
	inline void undo() { reverse(); }

private:
	std::function<void()> command;
	std::function<void()> reverse;
};

class UndoStack {
public:
	inline UndoStack(int maxCommands) : maxCommands(maxCommands) {};

	inline void execute(UndoableCommand command) {
		command.execute();

		undoStack.push_front(command);
		if (undoStack.size() > maxCommands) {
			undoStack.pop_back();
		}
	}

	inline void undo() {
		if (undoStack.size() > 0) {
			undoStack.front().undo();

			redoStack.push_front(undoStack.front());
			if (redoStack.size() > maxCommands) {
				redoStack.pop_back();
			}

			undoStack.pop_front();
		}
	}

	inline void redo() {
		if (redoStack.size() > 0) {
			redoStack.front().execute();

			undoStack.push_front(redoStack.front());
			if (undoStack.size() > maxCommands) {
				undoStack.pop_back();
			}
			
			redoStack.pop_front();
		}
	}

	inline void clear() {
		undoStack.clear();
		redoStack.clear();
	}

private:
	// Maximum number of commands stored in the UndoStack
	int maxCommands;

	// New commands pushed to front
	std::deque<UndoableCommand> undoStack;
	std::deque<UndoableCommand> redoStack;
};