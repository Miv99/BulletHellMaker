#include <DataStructs/UndoStack.h>

UndoableCommand::UndoableCommand(std::function<void()> command, std::function<void()> reverse) 
	: command(command), reverse(reverse) {
}

UndoStack::UndoStack(int maxCommands) 
	: maxCommands(maxCommands) {
}

void UndoStack::execute(UndoableCommand command) {
	command.execute();

	undoStack.push_front(command);
	if (undoStack.size() > maxCommands) {
		undoStack.pop_back();
	}
	redoStack.clear();
}

void UndoStack::undo() {
	if (undoStack.size() > 0) {
		undoStack.front().undo();

		redoStack.push_front(undoStack.front());
		if (redoStack.size() > maxCommands) {
			redoStack.pop_back();
		}

		undoStack.pop_front();
	}
}

void UndoStack::redo() {
	if (redoStack.size() > 0) {
		redoStack.front().execute();

		undoStack.push_front(redoStack.front());
		if (undoStack.size() > maxCommands) {
			undoStack.pop_back();
		}

		redoStack.pop_front();
	}
}

void UndoStack::clear() {
	undoStack.clear();
	redoStack.clear();
}