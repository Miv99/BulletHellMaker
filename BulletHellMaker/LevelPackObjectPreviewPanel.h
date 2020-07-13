#pragma once
#include "EditorUtilities.h"
#include "LevelPack.h"
#include "SpriteLoader.h"

class LevelPackObjectPreviewPanel : public SimpleEngineRenderer {
public:
	LevelPackObjectPreviewPanel(sf::RenderWindow& parentWindow, std::shared_ptr<LevelPack> levelPack, std::shared_ptr<SpriteLoader> spriteLoader);
	~LevelPackObjectPreviewPanel();
	static std::shared_ptr<LevelPackObjectPreviewPanel> create(sf::RenderWindow& parentWindow, std::shared_ptr<LevelPack> levelPack, std::shared_ptr<SpriteLoader> spriteLoader) {
		return std::make_shared<LevelPackObjectPreviewPanel>(parentWindow, levelPack, spriteLoader);
	}

private:
	void reloadPreview();
};