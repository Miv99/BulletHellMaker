#include <Editor/Windows/MainEditorWindowMenuBar.h>

#include <Config.h>

#include <Util/IOUtils.h>
#include <Editor/Windows/MainEditorWindow.h>

MainEditorWindowMenuBar::MainEditorWindowMenuBar(MainEditorWindow& mainEditorWindow) 
	: mainEditorWindow(mainEditorWindow) {

	addMenu("File");
	addMenuItem("File", "Open");
	connectMenuItem("File", "Open", [this]() {
		if (this->mainEditorWindow.hasUnsavedChanges()) {
			this->mainEditorWindow.promptConfirmation("Save all unsaved changes to this level pack?", this, true)->sink()
				.connect<MainEditorWindowMenuBar, &MainEditorWindowMenuBar::onOpenLevelPackWhileUnsavedChangesExistConfirmation>(this);
		} else {
			promptOpenLevelPack();
		}
	});
}

void MainEditorWindowMenuBar::promptOpenLevelPack() {
	char curDirectory[MAX_PATH + 1];
	GetCurrentDirectory(MAX_PATH + 1, curDirectory);
	strcat(curDirectory, ("\\" + RELATIVE_LEVEL_PACKS_FOLDER_PATH).c_str());
	int levelPackNameOffset = strlen(curDirectory) + 1;
	LPSTR result = new char[MAX_PATH + 1];
	strcpy(result, curDirectory);

	LPSTR title("Choose level pack folder");
	BrowseFolder(this->mainEditorWindow.getWindow()->getSystemHandle(), result, title);
	
	std::string resultStr(result);
	if (resultStr.size() > levelPackNameOffset) {
		std::string levelPackName = resultStr.substr(levelPackNameOffset);
		mainEditorWindow.loadLevelPack(levelPackName);
	}
}

void MainEditorWindowMenuBar::onOpenLevelPackWhileUnsavedChangesExistConfirmation(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE choice) {
	if (choice == EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE::YES) {
		mainEditorWindow.saveAllChanges();
		promptOpenLevelPack();
	} else if (choice == EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE::NO) {
		promptOpenLevelPack();
	}
	// Do nothing on cancel
}
