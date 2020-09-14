#include <Editor/SpriteSheets/SpriteSheetMetafileSpritesEditor.h>

#include <Editor/Windows/MainEditorWindow.h>

SpriteSheetMetafileSpritesEditor::SpriteSheetMetafileSpritesEditor(MainEditorWindow& mainEditorWindow) 
	: mainEditorWindow(mainEditorWindow) {
}

void SpriteSheetMetafileSpritesEditor::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	tgui::Panel::draw(target, states);
	// TODO
}

bool SpriteSheetMetafileSpritesEditor::handleEvent(sf::Event event) {
	// TODO
	return false;
}

void SpriteSheetMetafileSpritesEditor::loadImage(std::string fileName) {
	// TODO
}

void SpriteSheetMetafileSpritesEditor::resetCamera() {
	// TODO
}
