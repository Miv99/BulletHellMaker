#pragma once
#include <TGUI/TGUI.hpp>

#include <Util/IOUtils.h>
#include <DataStructs/SpriteLoader.h>
#include <Editor/EventCapturable.h>
#include <Editor/CustomWidgets/ListViewScrollablePanel.h>
#include <Editor/Windows/EditorWindowConfirmationPromptChoice.h>

class MainEditorWindow;
class LevelPack;

/*
A tgui::Panel containing a ListViewScrollablePanel that lists all loaded sprite sheets in a LevelPack.

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

	void selectSpriteSheetByName(std::string spriteSheetName);

	/*
	Reloads the current level pack's SpriteLoader's sprite sheets and
	then this widget's list of sprite sheets to match the SpriteLoader.
	*/
	SpriteLoader::LoadMetrics reloadSpriteLoaderAndList();

	/*
	Reloads the list of sprite sheets to match the loaded ones in
	the current level pack's SpriteLoader.
	*/
	void reloadListOnly();

	std::shared_ptr<ListViewScrollablePanel> getListViewScrollablePanel();
	/*
	Returns the name of the sprite sheet at some index
	in the list of sprite sheets.
	*/
	std::string getSpriteSheetNameByIndex(int index);

	void setLevelPack(LevelPack* levelPack);

private:
	struct SpriteSheetsTreeViewPanelReplaceFileData {
		std::string sourceImageFullPath;
		std::string destImageFullPath;

		bool sourceMetafileExists;
		std::string sourceMetafileFullPath;
		std::string destMetafileFullPath;
	};

	const static std::string SAVED_SPRITE_SHEET_ITEM_FORMAT;
	const static std::string UNSAVED_SPRITE_SHEET_ITEM_FORMAT;

	MainEditorWindow& mainEditorWindow;
	LevelPack* levelPack;

	// Contains sprite sheets, their sprites, and their animations
	std::shared_ptr<ListViewScrollablePanel> spriteSheetsList;

	// Maps sprite sheet name to index in spriteSheetsList
	std::map<std::string, int> spriteSheetIndexByName;
	// Maps index in spriteSheetsList to sprite sheet name
	std::map<int, std::string> spriteSheetNameByIndex;

	void importExternalSpriteSheet(SpriteSheetsTreeViewPanelReplaceFileData data);

	void onImportExternalSpriteSheetWhileFilesExistConfirmation(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE choice, 
		SpriteSheetsTreeViewPanelReplaceFileData dataObject);
};