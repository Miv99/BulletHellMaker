#pragma once
#include <string>
#include <vector>

#include <TGUI/TGUI.hpp>
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

#include <exprtk.hpp>
#include <LevelPack/TextMarshallable.h>
#include <LevelPack/LevelPackObject.h>
#include <DataStructs/SpriteLoader.h>
#include <LevelPack/LevelPack.h>
#include <Game/EntityCreationQueue.h>
#include <LevelPack/LevelPackObject.h>
#include <DataStructs/SymbolTable.h>
#include <LevelPack/ExpressionCompilable.h>

class LevelEvent : public LevelPackObject {
public:
	virtual std::string format() const = 0;
	virtual void load(std::string formattedString) = 0;

	virtual nlohmann::json toJson() = 0;
	virtual void load(const nlohmann::json& j) = 0;

	virtual std::shared_ptr<LevelPackObject> clone() const = 0;

	virtual std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const = 0;
	virtual void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) = 0;

	virtual void execute(SpriteLoader& spriteLoader, LevelPack& levelPack, entt::DefaultRegistry& registry, EntityCreationQueue& queue) = 0;
};

/*
A LevelEvent that spawns enemies.
*/
class SpawnEnemiesLevelEvent : public LevelEvent {
public:
	SpawnEnemiesLevelEvent();
	SpawnEnemiesLevelEvent(std::vector<std::shared_ptr<EnemySpawnInfo>> spawnInfo);

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	std::shared_ptr<LevelPackObject> clone() const override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

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
	enum class POSITION_ON_SCREEN {
		TOP = 0,
		BOTTOM = 1
	};

	NLOHMANN_JSON_SERIALIZE_ENUM(ROTATION_TYPE, {
		{ROTATION_TYPE::TOP, "TOP"},
		{ROTATION_TYPE::BOTTOM, "BOTTOM"}
	})

	NLOHMANN_JSON_SERIALIZE_ENUM(tgui::ShowAnimationType, {
		{tgui::ShowAnimationType::Fade, "Fade"},
		{tgui::ShowAnimationType::Scale, "Scale"},
		{tgui::ShowAnimationType::SlideToRight, "SlideToRight"},
		{tgui::ShowAnimationType::SlideToLeft, "SlideToLeft"},
		{tgui::ShowAnimationType::SlideToBottom, "SlideToBottom"},
		{tgui::ShowAnimationType::SlideToTop, "SlideToTop"},
		{tgui::ShowAnimationType::SlideFromLeft, "SlideFromLeft"},
		{tgui::ShowAnimationType::SlideFromRight, "SlideFromRight"},
		{tgui::ShowAnimationType::SlideFromTop, "SlideFromTop"},
		{tgui::ShowAnimationType::SlideFromBottom, "SlideFromBottom"},
	})

	ShowDialogueLevelEvent();
	ShowDialogueLevelEvent(std::string dialogueBoxTextureFileName, std::vector<std::string> text, POSITION_ON_SCREEN pos, tgui::ShowAnimationType showAnimation);

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	std::shared_ptr<LevelPackObject> clone() const override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	void execute(SpriteLoader& spriteLoader, LevelPack& levelPack, entt::DefaultRegistry& registry, EntityCreationQueue& queue) override;

	POSITION_ON_SCREEN getDialogueBoxPosition() { return dialogueBoxPosition; }
	tgui::ShowAnimationType getDialogueBoxShowAnimationType() { return dialogueBoxShowAnimationType; }
	float getDialogueBoxShowAnimationTime() { return dialogueBoxShowAnimationTime; }
	std::vector<std::string> getText() { return text; }
	std::string getDialogueBoxTextureFileName() { return dialogueBoxTextureFileName; }
	sf::IntRect getTextureMiddlePart() { return textureMiddlePart; }
	std::string getDialogueBoxPortraitFileName() { return dialogueBoxPortraitFileName; }

	void setDialogueBoxPosition(POSITION_ON_SCREEN dialogueBoxPosition) { this->dialogueBoxPosition = dialogueBoxPosition; }
	void setDialogueBoxShowAnimationType(tgui::ShowAnimationType dialogueBoxShowAnimationType) { this->dialogueBoxShowAnimationType = dialogueBoxShowAnimationType; }
	void setDialogueBoxShowAnimationTime(float dialogueBoxShowAnimationTime) { this->dialogueBoxShowAnimationTime = dialogueBoxShowAnimationTime; }
	void setText(std::vector<std::string> text) { this->text = text; }
	void setDialogueBoxTextureFileName(std::string dialogueBoxTextureFileName) { this->dialogueBoxTextureFileName = dialogueBoxTextureFileName; }
	void setTextureMiddlePart(sf::IntRect textureMiddlePart) { this->textureMiddlePart = textureMiddlePart; }
	void setDialogueBoxPortraitFileName(std::string dialogueBoxPortraitFileName) { this->dialogueBoxPortraitFileName = dialogueBoxPortraitFileName; }

private:
	POSITION_ON_SCREEN dialogueBoxPosition = POSITION_ON_SCREEN::BOTTOM;
	tgui::ShowAnimationType dialogueBoxShowAnimationType = tgui::ShowAnimationType::SlideFromBottom;
	// How long it takes to show the dialogue box
	float dialogueBoxShowAnimationTime = 0;
	// Each index in this vector is a separate dialogue box
	std::vector<std::string> text;

	std::string dialogueBoxTextureFileName;
	// The middle part of the dialogue box texture to be used for 9-slicing
	sf::IntRect textureMiddlePart = sf::IntRect();

	// Empty string if no portrait is to be used
	std::string dialogueBoxPortraitFileName = "";
};

class LevelEventFactory {
public:
	static std::shared_ptr<LevelEvent> create(std::string formattedString);
	static std::shared_ptr<LevelEvent> create(const nlohmann::json& j);
};