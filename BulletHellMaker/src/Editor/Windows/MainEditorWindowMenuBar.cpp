#include <Editor/Windows/MainEditorWindowMenuBar.h>

#include <Mutex.h>
#include <Config.h>
#include <Util/IOUtils.h>
#include <Editor/Windows/MainEditorWindow.h>

MainEditorWindowMenuBar::MainEditorWindowMenuBar(MainEditorWindow& mainEditorWindow) 
	: mainEditorWindow(mainEditorWindow) {

	std::lock_guard<std::recursive_mutex> lock(tguiMutex);

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

	addMenuItem("File", "Reload sprites/animations");
	connectMenuItem("File", "Reload sprites/animations", [this]() {
		this->mainEditorWindow.reloadSpriteLoader();
	});

	addMenu("Preview");
	addMenuItem("Preview", "Open window");
	connectMenuItem("Preview", "Open window", [this]() {
		this->mainEditorWindow.openPreviewWindow();
	});
}

void MainEditorWindowMenuBar::promptOpenLevelPack() {
	std::string curDirectory = getPathToFolderContainingExe() + "\\" + RELATIVE_LEVEL_PACKS_FOLDER_PATH;
	int levelPackNameOffset = curDirectory.size() + 1;
	LPSTR result = new char[MAX_PATH + 1];
	strcpy(result, curDirectory.c_str());

	LPSTR title("Choose level pack folder");
	BrowseFolder(this->mainEditorWindow.getWindow()->getSystemHandle(), result, title);
	
	std::string resultStr(result);
	if (resultStr.size() > levelPackNameOffset) {
		std::string levelPackName = resultStr.substr(levelPackNameOffset);
		mainEditorWindow.loadLevelPack(levelPackName);
	}

	delete[] result;
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
