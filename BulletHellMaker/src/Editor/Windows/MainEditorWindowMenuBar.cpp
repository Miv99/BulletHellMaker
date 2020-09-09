#include <Editor/Windows/MainEditorWindowMenuBar.h>

#include <Config.h>

#include <Util/IOUtils.h>
#include <Editor/Windows/MainEditorWindow.h>

MainEditorWindowMenuBar::MainEditorWindowMenuBar(MainEditorWindow& mainEditorWindow) 
	: mainEditorWindow(mainEditorWindow) {

	addMenu("File");
	addMenuItem("File", "Open");
	connectMenuItem("File", "Open", [this]() {
		char curDirectory[MAX_PATH + 1];
		GetCurrentDirectory(MAX_PATH + 1, curDirectory);
		strcat(curDirectory, ("\\" + RELATIVE_LEVEL_PACKS_FOLDER_PATH).c_str());
		int levelPackNameOffset = strlen(curDirectory) + 1;
		LPSTR result = new char[MAX_PATH + 1];
		strcpy(result, curDirectory);

		LPSTR title("Choose level pack folder");
		BrowseFolder(this->mainEditorWindow.getWindow()->getSystemHandle(), result, title);
		std::string fullLevelPackPath(result);

		this->mainEditorWindow.loadLevelPack(fullLevelPackPath.substr(levelPackNameOffset));
	});
}