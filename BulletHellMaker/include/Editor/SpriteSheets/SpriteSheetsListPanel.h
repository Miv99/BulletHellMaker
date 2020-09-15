#pragma once
#include <TGUI/TGUI.hpp>

#include <Util/IOUtils.h>
#include <Editor/EventCapturable.h>
#include <Editor/CustomWidgets/ListViewScrollablePanel.h>
#include <Editor/Windows/EditorWindowConfirmationPromptChoice.h>

class MainEditorWindow;
class LevelPack;

/*
A tgui::Panel containing a ListViewScrollablePanel that lists all sprite sheets in a LevelPack.

Doesn't support CopyPaste operations.
*/
class SpriteSheetsListPanel : public tgui::Panel {
public:
	SpriteSheetsListPanel(MainEditorWindow& mainEditorWindow);
	static std::shared_ptr<SpriteSheetsListPanel> create(MainEditorWindow& mainEditorWindow) {
		return std::make_shared<SpriteSheetsListPanel>(mainEditorWindow);
	}

	/*
	Opens a prompt for the user to import an external sprite sheet.
	*/
	void promptImportExternalSpriteSheet();

	/*
	Reloads the current level pack's SpriteLoader's sprite sheets and
	then this widget's list of sprite sheets to match the SpriteLoader.
	*/
	void reloadSpriteLoaderAndList();

	/*
	Reloads the list of sprite sheets to match the loaded ones in
	the current level pack's SpriteLoader.
	*/
	void reloadListOnly();

	std::shared_ptr<ListViewScrollablePanel> getListView();

	void setLevelPack(LevelPack* levelPack);

private:
	struct SpriteSheetsTreeViewPanelReplaceFileData {
		std::string sourceImageFullPath;
		std::string destImageFullPath;

		bool sourceMetafileExists;
		std::string sourceMetafileFullPath;
		std::string destMetafileFullPath;
	};

	MainEditorWindow& mainEditorWindow;
	LevelPack* levelPack;

	// Contains sprite sheets, their sprites, and their animations
	std::shared_ptr<ListViewScrollablePanel> spriteSheetsList;

	void importExternalSpriteSheet(SpriteSheetsTreeViewPanelReplaceFileData data);

	void onImportExternalSpriteSheetWhileFilesExistConfirmation(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE choice, 
		SpriteSheetsTreeViewPanelReplaceFileData dataObject);
};