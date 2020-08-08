#pragma once
#include <vector>
#include <utility>
#include <string>
#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>
#include <memory>
#include "LevelPackObject.h"
#include "SymbolTable.h"
#include "exprtk.hpp"
#include "LevelEventStartCondition.h"
#include "EnemySpawn.h"
#include "TextMarshallable.h"
#include "Item.h"
#include "LevelEvent.h"
#include "RenderSystem.h"
#include "AudioPlayer.h"
#include "ExpressionCompilable.h"

/*
This is a top-level object so every expression this uses should be in terms of only its own unredelegated, well-defined symbols
meaning every symbol in this object's symbol table is not redelegated.
*/
class Level : public LevelPackObject {
public:
	inline Level() {}
	inline Level(std::string name) : name(name) {}

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	/*
	Execute the LevelEvent at index eventIndex.
	*/
	inline void executeEvent(int eventIndex, SpriteLoader& spriteLoader, LevelPack& levelPack, entt::DefaultRegistry& registry, EntityCreationQueue& queue) {
		events[eventIndex].second->execute(spriteLoader, levelPack, registry, queue);
	}

	inline std::string getName() const { return name; }
	inline int getEventsCount() const { return events.size(); }
	inline std::shared_ptr<HealthPackItem> getHealthPack() {
		if (!healthPack) {
			healthPack = std::make_shared<HealthPackItem>();
		}
		return healthPack;
	}
	inline std::shared_ptr<PointsPackItem> getPointsPack() {
		if (!pointPack) {
			pointPack = std::make_shared<PointsPackItem>();
		}
		return pointPack;
	}
	inline std::shared_ptr<PowerPackItem> getPowerPack() {
		if (!powerPack) {
			powerPack = std::make_shared<PowerPackItem>();
		}
		return powerPack;
	}
	inline std::shared_ptr<BombItem> getBombItem() {
		if (!bombItem) {
			bombItem = std::make_shared<BombItem>();
		}
		return bombItem;
	}

	inline MusicSettings getMusicSettings() { return musicSettings; }
	inline std::string getBackgroundFileName() const { return backgroundFileName; }
	inline float getBackgroundScrollSpeedX() const { return backgroundScrollSpeedX; }
	inline float getBackgroundScrollSpeedY() const { return backgroundScrollSpeedY; }
	inline sf::Color getBossNameColor() const { return bossNameColor; }
	inline sf::Color getBossHPBarColor() const { return bossHPBarColor; }
	inline float getBackgroundTextureWidth() const { return backgroundTextureWidth; }
	inline float getBackgroundTextureHeight() const { return backgroundTextureHeight; }
	inline bool usesEnemy(int enemyID) const { return enemyIDCount.count(enemyID) > 0 && enemyIDCount.at(enemyID) > 0; }

	inline void setMusicSettings(MusicSettings musicSettings) { this->musicSettings = musicSettings; }
	inline void setName(std::string name) { this->name = name; }
	inline void setBackgroundFileName(std::string backgroundFileName) { this->backgroundFileName = backgroundFileName; }
	inline void setBackgroundScrollSpeedX(float backgroundScrollSpeedX) { this->backgroundScrollSpeedX = backgroundScrollSpeedX; }
	inline void setBackgroundScrollSpeedY(float backgroundScrollSpeedY) { this->backgroundScrollSpeedY = backgroundScrollSpeedY; }
	inline void setBossNameColor(sf::Color bossNameColor) { this->bossNameColor = bossNameColor; }
	inline void setBossHPBarColor(sf::Color bossHPBarColor) { this->bossHPBarColor = bossHPBarColor; }
	inline float setBackgroundTextureWidth(float backgroundTextureWidth) { this->backgroundTextureWidth = backgroundTextureWidth; }
	inline float setBackgroundTextureHeight(float backgroundTextureHeight) { this->backgroundTextureHeight = backgroundTextureHeight; }

	/*
	Insert a LevelEvent at index eventIndex.

	startCondition - the start condition of the event
	event - the LevelEvent
	*/
	void insertEvent(int eventIndex, std::shared_ptr<LevelEventStartCondition> startCondition, std::shared_ptr<LevelEvent> event);
	/*
	Remove the LevelEvent at index eventIndex.
	*/
	void removeEvent(int eventIndex);

	/*
	Returns whether the start condition for the LevelEvent at index conditionIndex has been satisfied.
	*/
	inline bool conditionSatisfied(int conditionIndex, entt::DefaultRegistry& registry) const { return events[conditionIndex].first->satisfied(registry); }

private:
	// Name of the level
	std::string name;

	// Sorted ascending by time of occurrence
	// The next start condition cannot be satisfied until the previous one is
	std::vector<std::pair<std::shared_ptr<LevelEventStartCondition>, std::shared_ptr<LevelEvent>>> events;

	std::shared_ptr<HealthPackItem> healthPack;
	std::shared_ptr<PointsPackItem> pointPack;
	std::shared_ptr<PowerPackItem> powerPack;
	std::shared_ptr<BombItem> bombItem;

	// Music to play on start of level
	MusicSettings musicSettings;

	std::string backgroundFileName;
	float backgroundScrollSpeedX = 0;
	float backgroundScrollSpeedY = 0;
	// How many pixels in the background image will be shown on screen at any given time
	float backgroundTextureWidth = MAP_WIDTH;
	float backgroundTextureHeight = MAP_HEIGHT;

	// Boss name and hp bar colors are in Level instead of in Enemy because they are drawn in the playing area, 
	// on top of the background, so they should be up to Level to decide
	sf::Color bossNameColor = sf::Color::White;
	sf::Color bossHPBarColor = sf::Color::Red;

	// Maps an EditorEnemy ID to the number of times it will be spawned in events.
	// This is not saved on format() but is reconstructed in load().
	std::map<int, int> enemyIDCount;
};