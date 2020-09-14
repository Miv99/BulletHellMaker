#include <Editor/SpriteSheets/SpriteSheetsListPanel.h>

#include <tchar.h>

#include <Config.h>
#include <GuiConfig.h>
#include <Editor/Util/EditorUtils.h>
#include <Editor/Windows/MainEditorWindow.h>
#include <LevelPack/LevelPack.h>

SpriteSheetsListPanel::SpriteSheetsListPanel(MainEditorWindow& mainEditorWindow) 
	: mainEditorWindow(mainEditorWindow), levelPack(nullptr) {
	
	// TODO

	std::shared_ptr<tgui::Button> importSpriteSheetButton = tgui::Button::create();
	importSpriteSheetButton->setPosition(GUI_PADDING_X, GUI_PADDING_Y);
	importSpriteSheetButton->setSize(SMALL_BUTTON_SIZE, SMALL_BUTTON_SIZE);
	importSpriteSheetButton->connect("Pressed", [this]() {
		promptImportExternalSpriteSheet();
	});
	importSpriteSheetButton->setToolTip(createToolTip("Imports a sprite sheet image from outside the level pack. \
If there is a corresponding BulletHellMaker sprite sheet metafile (it should be named \
\"[SpriteSheetImageFileNameAndExtension].txt\"), it should be in the same folder as the image."));
	add(importSpriteSheetButton);

	spriteSheetsList = ListViewScrollablePanel::create();
	spriteSheetsList->setPosition(tgui::bindLeft(importSpriteSheetButton), tgui::bindBottom(importSpriteSheetButton));
	add(spriteSheetsList);
	
	connect("SizeChanged", [this, importSpriteSheetButton](sf::Vector2f newSize) {
		spriteSheetsList->setSize(newSize.x - GUI_PADDING_X * 2, newSize.y - GUI_PADDING_Y - tgui::bindBottom(importSpriteSheetButton));
	});
}

bool SpriteSheetsListPanel::handleEvent(sf::Event event) {
	// TODO
	return false;
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
	ofn.lpstrFilter = "Image\0*.bmp;*.png;*.tga;*.jpg;*.gif;*.psd;*.hdr;*.pic\0\0";
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

void SpriteSheetsListPanel::reload() {
	std::shared_ptr<tgui::ListView> listView = spriteSheetsList->getListView();

	listView->removeAllItems();
	for (std::string spriteSheetName : levelPack->getSpriteLoader()->getLoadedSpriteSheetNames()) {
		listView->addItem(spriteSheetName);
	}
}

void SpriteSheetsListPanel::setLevelPack(LevelPack* levelPack) {
	this->levelPack = levelPack;
	reload();
}

void SpriteSheetsListPanel::importExternalSpriteSheet(SpriteSheetsTreeViewPanelReplaceFileData data) {
	if (data.sourceImageFullPath == data.destImageFullPath) {
		// Nothing to do
		return;
	}

	BOOL copyIsSuccessful = CopyFile(data.sourceImageFullPath.c_str(), data.destImageFullPath.c_str(), FALSE);
	if (!copyIsSuccessful) {
		// TODO: log error code
		// TODO: show error code to user through MainEditorWindow prompt
		DWORD errorCode = GetLastError();
		assert(false);
		return;
	}

	if (data.sourceMetafileExists) {
		BOOL metafileCopyIsSuccessful = CopyFile(data.sourceMetafileFullPath.c_str(), data.destMetafileFullPath.c_str(), FALSE);
		if (!metafileCopyIsSuccessful) {
			// TODO: log error code
			// TODO: show error code to user through MainEditorWindow prompt
			DWORD errorCode = GetLastError();
			assert(false);
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
