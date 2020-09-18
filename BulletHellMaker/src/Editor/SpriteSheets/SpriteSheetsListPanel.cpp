#include <Editor/SpriteSheets/SpriteSheetsListPanel.h>

#include <tchar.h>

#include <Mutex.h>
#include <Config.h>
#include <GuiConfig.h>
#include <Editor/Util/EditorUtils.h>
#include <Editor/Windows/MainEditorWindow.h>
#include <LevelPack/LevelPack.h>

// %s = the sprite sheet's name
const std::string SpriteSheetsListPanel::SAVED_SPRITE_SHEET_ITEM_FORMAT = "%s";
const std::string SpriteSheetsListPanel::UNSAVED_SPRITE_SHEET_ITEM_FORMAT = "*%s";

SpriteSheetsListPanel::SpriteSheetsListPanel(MainEditorWindow& mainEditorWindow) 
	: mainEditorWindow(mainEditorWindow), levelPack(nullptr) {

	std::lock_guard<std::recursive_mutex> lock(tguiMutex);
	
	std::shared_ptr<tgui::Button> importSpriteSheetButton = tgui::Button::create();
	importSpriteSheetButton->setTextSize(TEXT_SIZE);
	importSpriteSheetButton->setText("+");
	importSpriteSheetButton->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	importSpriteSheetButton->setSize(SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE);
	importSpriteSheetButton->connect("Pressed", [this]() {
		promptImportExternalSpriteSheet();
	});
	importSpriteSheetButton->setToolTip(createToolTip("Imports a sprite sheet image from outside the level pack. \
If there is a corresponding BulletHellMaker sprite sheet metafile (named \
\"[SpriteSheetImageFileNameAndExtension].txt\"), it should be in the same folder as the image."));
	add(importSpriteSheetButton);

	spriteSheetsList = ListViewScrollablePanel::create();
	spriteSheetsList->setPosition(tgui::bindLeft(importSpriteSheetButton), tgui::bindBottom(importSpriteSheetButton));
	add(spriteSheetsList);
	
	connect("SizeChanged", [this, importSpriteSheetButton](sf::Vector2f newSize) {
		spriteSheetsList->setSize(newSize.x - GUI_PADDING_X * 2, newSize.y - GUI_PADDING_Y - tgui::bindBottom(importSpriteSheetButton));
	});
}

void SpriteSheetsListPanel::promptImportExternalSpriteSheet() {
	OPENFILENAME ofn;
	char sourceImageFullPath[MAX_PATH + 1];

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = this->mainEditorWindow.getWindow()->getSystemHandle();
	ofn.lpstrFile = sourceImageFullPath;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(sourceImageFullPath);
	ofn.lpstrFilter = "Image (*.bmp; *.png; *.tga; *.jpg; *.gif; *.psd; *.hdr; *.pic)\0*.bmp;*.png;*.tga;*.jpg;*.gif;*.psd;*.hdr;*.pic\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	GetOpenFileNameA(&ofn);

	if (strlen(sourceImageFullPath) <= 0) {
		return;
	}

	char sourceMetafileFullPath[MAX_PATH + 1];
	strcpy(sourceMetafileFullPath, sourceImageFullPath);
	strcat(sourceMetafileFullPath, ".txt");
	bool sourceMetafileExists = fileExists(sourceMetafileFullPath);

	char imageFileName[MAX_PATH + 1];
	char imageFileExtension[MAX_PATH + 1];
	_splitpath(sourceImageFullPath, NULL, NULL, imageFileName, imageFileExtension);

	if (imageExtensionIsSupportedBySFML(imageFileExtension)) {
		std::string curDirectory = getPathToFolderContainingExe();

		std::string destImageFullPath = format(curDirectory + "\\" + RELATIVE_LEVEL_PACK_SPRITE_SHEETS_FOLDER_PATH + "\\%s%s", levelPack->getName().c_str(), imageFileName, imageFileExtension);
		std::string destMetafileFullPath = format(curDirectory + "\\" + RELATIVE_LEVEL_PACK_SPRITE_SHEETS_FOLDER_PATH + "\\%s%s.txt", levelPack->getName().c_str(), imageFileName, imageFileExtension);

		SpriteSheetsTreeViewPanelReplaceFileData data = { std::string(sourceImageFullPath), destImageFullPath, sourceMetafileExists, std::string(sourceMetafileFullPath), destMetafileFullPath };
		if ((fileExists(destImageFullPath) || (fileExists(destMetafileFullPath) && sourceMetafileExists)) && std::string(sourceImageFullPath) != destImageFullPath) {
			this->mainEditorWindow.promptConfirmation("The sprite sheet and/or its metafile already exist in this level pack. Replace them?", data, this, false)->sink()
				.connect<SpriteSheetsListPanel, &SpriteSheetsListPanel::onImportExternalSpriteSheetWhileFilesExistConfirmation>(this);
		} else {
			importExternalSpriteSheet(data);
		}
	}
}

void SpriteSheetsListPanel::selectSpriteSheetByName(std::string spriteSheetName) {
	spriteSheetsList->getListView()->setSelectedItem(spriteSheetIndexByName[spriteSheetName]);
}

SpriteLoader::LoadMetrics SpriteSheetsListPanel::reloadSpriteLoaderAndList() {
	SpriteLoader::LoadMetrics loadMetrics = levelPack->getSpriteLoader()->loadFromSpriteSheetsFolder();
	reloadListOnly();
	return loadMetrics;
}

void SpriteSheetsListPanel::reloadListOnly() {
	spriteSheetIndexByName.clear();
	spriteSheetNameByIndex.clear();
	std::shared_ptr<tgui::ListView> listView = spriteSheetsList->getListView();

	listView->removeAllItems();

	const std::map<std::string, std::shared_ptr<SpriteSheet>>& unsavedSpriteSheets = mainEditorWindow.getUnsavedSpriteSheets();
	int i = 0;
	for (std::string spriteSheetName : levelPack->getSpriteLoader()->getLoadedSpriteSheetNames()) {
		if (unsavedSpriteSheets.find(spriteSheetName) == unsavedSpriteSheets.end()) {
			listView->addItem(format(SAVED_SPRITE_SHEET_ITEM_FORMAT, spriteSheetName.c_str()));
		} else {
			listView->addItem(format(UNSAVED_SPRITE_SHEET_ITEM_FORMAT, spriteSheetName.c_str()));
		}
		spriteSheetIndexByName[spriteSheetName] = i;
		spriteSheetNameByIndex[i] = spriteSheetName;
		i++;
	}
}

std::shared_ptr<ListViewScrollablePanel> SpriteSheetsListPanel::getListViewScrollablePanel() {
	return spriteSheetsList;
}

std::string SpriteSheetsListPanel::getSpriteSheetNameByIndex(int index) {
	// Each item in the list view is the file name with extension, and
	// the name of a sprite sheet is the same as the image file name (see SpriteLoader)
	return spriteSheetNameByIndex.at(index);
}

void SpriteSheetsListPanel::setLevelPack(LevelPack* levelPack) {
	this->levelPack = levelPack;
	reloadListOnly();
}

void SpriteSheetsListPanel::importExternalSpriteSheet(SpriteSheetsTreeViewPanelReplaceFileData data) {
	if (data.sourceImageFullPath == data.destImageFullPath) {
		// Nothing to do
		return;
	}

	BOOL copyIsSuccessful = CopyFile(data.sourceImageFullPath.c_str(), data.destImageFullPath.c_str(), FALSE);
	if (!copyIsSuccessful) {
		// TODO: log error code

		DWORD errorCode = GetLastError();
		showWindowsErrorDialog(errorCode, (LPCWSTR)L"Error copying image file");

		return;
	}

	if (data.sourceMetafileExists) {
		BOOL metafileCopyIsSuccessful = CopyFile(data.sourceMetafileFullPath.c_str(), data.destMetafileFullPath.c_str(), FALSE);

		if (!metafileCopyIsSuccessful) {
			// TODO: log error code

			DWORD errorCode = GetLastError();
			showWindowsErrorDialog(errorCode, (LPCWSTR)L"Error copying image metafile");

			return;
		}

		char metafileFileNameAndExtension[MAX_PATH + 1];
		char metafileExtension[MAX_PATH + 1];
		_splitpath(data.destMetafileFullPath.c_str(), NULL, NULL, metafileFileNameAndExtension, metafileExtension);
		// Metafile name without the extension is the image file name and extension
		std::string imageFileNameAndExtension(metafileFileNameAndExtension);
		strcat(metafileFileNameAndExtension, metafileExtension);
		levelPack->getSpriteLoader()->loadSpriteSheet(std::string(metafileFileNameAndExtension), imageFileNameAndExtension);
	}
}

void SpriteSheetsListPanel::onImportExternalSpriteSheetWhileFilesExistConfirmation(EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE choice,
	SpriteSheetsTreeViewPanelReplaceFileData data) {

	if (choice == EDITOR_WINDOW_CONFIRMATION_PROMPT_CHOICE::YES) {
		importExternalSpriteSheet(data);
	}
	// Do nothing if choice was No
}
