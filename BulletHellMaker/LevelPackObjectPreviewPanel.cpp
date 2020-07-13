#include "LevelPackObjectPreviewPanel.h"

LevelPackObjectPreviewPanel::LevelPackObjectPreviewPanel(sf::RenderWindow& parentWindow, std::shared_ptr<LevelPack> levelPack, std::shared_ptr<SpriteLoader> spriteLoader) 
	: SimpleEngineRenderer(parentWindow, true, false) {
	loadLevelPack(levelPack, spriteLoader);

	levelPack->getOnChange()->sink().connect<LevelPackObjectPreviewPanel, &LevelPackObjectPreviewPanel::reloadPreview>(this);
}

LevelPackObjectPreviewPanel::~LevelPackObjectPreviewPanel() {
	levelPack->getOnChange()->sink().disconnect(this);
}

void LevelPackObjectPreviewPanel::reloadPreview() {
	// TODO
}
