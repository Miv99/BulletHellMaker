#pragma once
#include <vector>
#include <utility>
#include <string>
#include <entt/entt.hpp>
#include <SFML/Graphics.hpp>
#include <memory>
#include "EnemySpawnCondition.h"
#include "EnemySpawn.h"
#include "TextMarshallable.h"
#include "Item.h"
#include "RenderSystem.h"
#include "AudioPlayer.h"

class Level : public TextMarshallable {
public:
	inline Level() {}
	inline Level(std::string name) : name(name) {}

	std::string format() const override;
	void load(std::string formattedString) override;

	bool legal(std::string& message) const;

	inline std::string getName() const { return name; }
	inline const std::vector<EnemySpawnInfo>& getEnemyGroupSpawnInfo(int conditionIndex) const { return enemyGroups[conditionIndex].second; }
	inline int getEnemyGroupsCount() const { return enemyGroups.size(); }
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
	/*
	Returns a reference to the music settings.
	*/
	inline MusicSettings& getMusicSettings() { return musicSettings; }
	inline std::string getBackgroundFileName() const { return backgroundFileName; }
	inline float getBackgroundScrollSpeedX() const { return backgroundScrollSpeedX; }
	inline float getBackgroundScrollSpeedY() const { return backgroundScrollSpeedY; }
	inline sf::Color getBossNameColor() const { return bossNameColor; }
	inline sf::Color getBossHPBarColor() const { return bossHPBarColor; }
	// Returns a reference
	inline std::vector<BloomSettings> getBloomLayerSettings() const { return bloomLayerSettings; }
	inline float getBackgroundTextureWidth() const { return backgroundTextureWidth; }
	inline float getBackgroundTextureHeight() const { return backgroundTextureHeight; }

	inline void setName(std::string name) { this->name = name; }
	inline void setBackgroundFileName(std::string backgroundFileName) { this->backgroundFileName = backgroundFileName; }
	inline void setBackgroundScrollSpeedX(float backgroundScrollSpeedX) { this->backgroundScrollSpeedX = backgroundScrollSpeedX; }
	inline void setBackgroundScrollSpeedY(float backgroundScrollSpeedY) { this->backgroundScrollSpeedY = backgroundScrollSpeedY; }
	inline void setBossNameColor(sf::Color bossNameColor) { this->bossNameColor = bossNameColor; }
	inline void setBossHPBarColor(sf::Color bossHPBarColor) { this->bossHPBarColor = bossHPBarColor; }
	inline float setBackgroundTextureWidth(float backgroundTextureWidth) { this->backgroundTextureWidth = backgroundTextureWidth; }
	inline float setBackgroundTextureHeight(float backgroundTextureHeight) { this->backgroundTextureHeight = backgroundTextureHeight; }

	// Inserts a spawn condition and enemies such that the new condition and enemies are at the specified index
	inline void insertEnemySpawns(int conditionIndex, std::shared_ptr<EnemySpawnCondition> spawnCondition, std::vector<EnemySpawnInfo> enemies) {
		enemyGroups.insert(enemyGroups.begin() + conditionIndex, std::make_pair(spawnCondition, enemies));

		for (auto& enemy : enemies) {
			int enemyID = enemy.getEnemyID();
			if (enemyIDCount.count(enemyID) == 0) {
				enemyIDCount[enemyID] = 1;
			} else {
				enemyIDCount[enemyID]++;
			}
		}
	}
	// Adds an enemy into an already existing spawn condition
	inline void addEnemy(int conditionIndex, EnemySpawnInfo enemy) {
		enemyGroups[conditionIndex].second.push_back(enemy);

		int enemyID = enemy.getEnemyID();
		if (enemyIDCount.count(enemyID) == 0) {
			enemyIDCount[enemyID] = 1;
		} else {
			enemyIDCount[enemyID]++;
		}
	}

	inline bool conditionSatisfied(int conditionIndex, entt::DefaultRegistry& registry) const { return enemyGroups[conditionIndex].first->satisfied(registry); }

private:
	// Name of the level
	std::string name;

	// Enemy spawns and when they appear (t=0 is start of level)
	// Multiple enemy spawns can depend on a single enemy spawn condition (eg spawn 5 enemies if enemyCount == 0) 
	// Sorted ascending by time of occurrence
	// The next spawn condition cannot be satisfied until the previous one is
	std::vector<std::pair<std::shared_ptr<EnemySpawnCondition>, std::vector<EnemySpawnInfo>>> enemyGroups;

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

	// Bloom settings for the level; each index is a separate layer
	std::vector<BloomSettings> bloomLayerSettings = std::vector<BloomSettings>(HIGHEST_RENDER_LAYER + 1, BloomSettings());

	// Maps an EditorEnemy ID to the number of times it will be spawned in enemyGroups.
	// This is not saved on format() but is reconstructed in load().
	std::map<int, int> enemyIDCount;
};