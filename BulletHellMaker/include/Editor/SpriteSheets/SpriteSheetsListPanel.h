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
class SpriteSheetsListPanel : public tgui::Panel, public EventCapturable {
public:
	SpriteSheetsListPanel(MainEditorWindow& mainEditorWindow);
	static std::shared_ptr<SpriteSheetsListPanel> create(MainEditorWindow& mainEditorWindow) {
		return std::make_shared<SpriteSheetsListPanel>(mainEditorWindow);
	}

	bool handleEvent(sf::Event event) override;

	void promptImportExternalSpriteSheet();

	void reload();

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