#include "EditorInstance.h"

EditorInstance::EditorInstance() {
	// testing
	attackEditorWindow = std::make_shared<AttackEditorWindow>(1024, 768);
	attackEditorThread = std::thread(&AttackEditorWindow::start, attackEditorWindow);
	attackEditorThread.join();
}
