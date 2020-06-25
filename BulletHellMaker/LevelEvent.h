#pragma once
#include "TextMarshallable.h"
#include "LevelPackObject.h"
#include "SpriteLoader.h"
#include "LevelPack.h"
#include "EntityCreationQueue.h"
#include "LevelPackObject.h"
#include "SymbolTable.h"
#include "exprtk.hpp"
#include <entt/entt.hpp>
#include <TGUI/TGUI.hpp>
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>

class LevelEvent : public TextMarshallable, public LevelPackObject {
public:
	std::string format() const = 0;
	void load(std::string formattedString) = 0;

	virtual std::shared_ptr<LevelPackObject> clone() const = 0;

	virtual std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const = 0;
	virtual void compileExpressions(exprtk::symbol_table<float> symbolTable) = 0;

	virtual void execute(SpriteLoader& spriteLoader, LevelPack& levelPack, entt::DefaultRegistry& registry, EntityCreationQueue& queue) = 0;
};

/*
A LevelEvent that spawns enemies.
*/
class SpawnEnemiesLevelEvent : public LevelEvent {
public:
	inline SpawnEnemiesLevelEvent() {}
	inline SpawnEnemiesLevelEvent(std::vector<std::shared_ptr<EnemySpawnInfo>> spawnInfo) : spawnInfo(spawnInfo) {}

	std::string format() const override;
	void load(std::string formattedString) override;

	std::shared_ptr<LevelPackObject> clone() const override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const override;
	void compileExpressions(exprtk::symbol_table<float> symbolTable) override;

	void execute(SpriteLoader& spriteLoader, LevelPack& levelPack, entt::DefaultRegistry& registry, EntityCreationQueue& queue) override;

	inline const std::vector<std::shared_ptr<EnemySpawnInfo>>& getSpawnInfo() { return spawnInfo; }

private:
	std::vector<std::shared_ptr<EnemySpawnInfo>> spawnInfo;
};

/*
A LevelEvent that shows dialogue to the user.
The dialogue box that is shown will not have a portrait.
*/
class ShowDialogueLevelEvent : public LevelEvent {
public:
	enum class PositionOnScreen {
		TOP = 0,
		BOTTOM = 1
	};

	inline ShowDialogueLevelEvent() {}
	inline ShowDialogueLevelEvent(std::string dialogueBoxTextureFileName, std::vector<std::string> text, PositionOnScreen pos, tgui::ShowAnimationType showAnimation) : text(text),
		dialogueBoxPosition(pos), dialogueBoxShowAnimationType(showAnimation), dialogueBoxTextureFileName(dialogueBoxTextureFileName) {
	}

	std::string format() const override;
	void load(std::string formattedString) override;

	std::shared_ptr<LevelPackObject> clone() const override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const;
	void compileExpressions(exprtk::symbol_table<float> symbolTable) override;

	void execute(SpriteLoader& spriteLoader, LevelPack& levelPack, entt::DefaultRegistry& registry, EntityCreationQueue& queue) override;

	PositionOnScreen getDialogueBoxPosition() { return dialogueBoxPosition; }
	tgui::ShowAnimationType getDialogueBoxShowAnimationType() { return dialogueBoxShowAnimationType; }
	float getDialogueBoxShowAnimationTime() { return dialogueBoxShowAnimationTime; }
	std::vector<std::string> getText() { return text; }
	std::string getDialogueBoxTextureFileName() { return dialogueBoxTextureFileName; }
	sf::IntRect getTextureMiddlePart() { return textureMiddlePart; }
	std::string getDialogueBoxPortraitFileName() { return dialogueBoxPortraitFileName; }

	void setDialogueBoxPosition(PositionOnScreen dialogueBoxPosition) { this->dialogueBoxPosition = dialogueBoxPosition; }
	void setDialogueBoxShowAnimationType(tgui::ShowAnimationType dialogueBoxShowAnimationType) { this->dialogueBoxShowAnimationType = dialogueBoxShowAnimationType; }
	void setDialogueBoxShowAnimationTime(float dialogueBoxShowAnimationTime) { this->dialogueBoxShowAnimationTime = dialogueBoxShowAnimationTime; }
	void setText(std::vector<std::string> text) { this->text = text; }
	void setDialogueBoxTextureFileName(std::string dialogueBoxTextureFileName) { this->dialogueBoxTextureFileName = dialogueBoxTextureFileName; }
	void setTextureMiddlePart(sf::IntRect textureMiddlePart) { this->textureMiddlePart = textureMiddlePart; }
	void setDialogueBoxPortraitFileName(std::string dialogueBoxPortraitFileName) { this->dialogueBoxPortraitFileName = dialogueBoxPortraitFileName; }

private:
	PositionOnScreen dialogueBoxPosition = PositionOnScreen::BOTTOM;
	tgui::ShowAnimationType dialogueBoxShowAnimationType = tgui::ShowAnimationType::SlideFromBottom;
	// How long it takes to show the dialogue box
	float dialogueBoxShowAnimationTime = 0;
	// Each index in this vector is a separate dialogue box
	std::vector<std::string> text;

	std::string dialogueBoxTextureFileName;
	// The middle part of the dialogue box texture to be used for 9-slicing
	sf::IntRect textureMiddlePart;

	// Empty string if no portrait is to be used
	std::string dialogueBoxPortraitFileName = "";
};

class LevelEventFactory {
public:
	static std::shared_ptr<LevelEvent> create(std::string formattedString);
};