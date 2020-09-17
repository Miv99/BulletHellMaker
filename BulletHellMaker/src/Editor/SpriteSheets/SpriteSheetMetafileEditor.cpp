#include <Editor/SpriteSheets/SpriteSheetMetafileEditor.h>

#include <Mutex.h>
#include <GuiConfig.h>
#include <Editor/Windows/MainEditorWindow.h>

SpriteSheetMetafileEditor::SpriteSheetMetafileEditor(MainEditorWindow& mainEditorWindow, Clipboard& clipboard, std::shared_ptr<SpriteSheet> spriteSheet)
	: mainEditorWindow(mainEditorWindow), clipboard(clipboard), spriteSheet(spriteSheet), CopyPasteable(SPRITE_SHEET_METAFILE_SPRITE_COPY_PASTE_ID) {

	std::lock_guard<std::recursive_mutex> lock(tguiMutex);
}

void SpriteSheetMetafileEditor::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	tgui::Panel::draw(target, states);
	// TODO
}

CopyOperationResult SpriteSheetMetafileEditor::copyFrom() {
	// TODO
	return CopyOperationResult(nullptr, "");
}

PasteOperationResult SpriteSheetMetafileEditor::pasteInto(std::shared_ptr<CopiedObject> pastedObject) {
	// TODO
	return PasteOperationResult(false, "");
}

PasteOperationResult SpriteSheetMetafileEditor::paste2Into(std::shared_ptr<CopiedObject> pastedObject) {
	// TODO
	return PasteOperationResult(false, "");
}

bool SpriteSheetMetafileEditor::handleEvent(sf::Event event) {
	// TODO
	return false;
}

void SpriteSheetMetafileEditor::loadImage(std::string fileName) {
	// TODO
}

void SpriteSheetMetafileEditor::resetCamera() {
	// TODO
}

tgui::Signal& SpriteSheetMetafileEditor::getSignal(std::string signalName) {
	if (signalName == tgui::toLower(onMetafileModify.getName())) {
		return onMetafileModify;
	}
	return tgui::Panel::getSignal(signalName);
}
