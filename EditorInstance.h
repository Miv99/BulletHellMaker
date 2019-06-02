#pragma once
#include <thread>
#include <memory>
#include "AttackEditor.h"

class EditorInstance {
public:
	//TODO: make a window that's like a main menu of editors; all editor windows are closed when EditorInstance is closed
	EditorInstance();

private:
	std::shared_ptr<AttackEditorWindow> attackEditorWindow;
	std::thread attackEditorThread;
};