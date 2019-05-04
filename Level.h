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
#include "AudioPlayer.h"

class Level : public TextMarshallable {
public:
	inline Level() {}
	inline Level(std::string name) : name(name) {}

	std::string format() override;
	void load(std::string formattedString) override;

	bool legal(std::string& message);

	inline std::string getName() { return name; }
	inline const std::vector<EnemySpawnInfo>& getEnemyGroupSpawnInfo(int conditionIndex) { return enemyGroups[conditionIndex].second; }
	inline int getEnemyGroupsCount() { return enemyGroups.size(); }
	inline std::shared_ptr<HealthPackItem> getHealthPack() { return healthPack; }
	inline std::shared_ptr<PointsPackItem> getPointsPack() { return pointPack; }
	inline std::shared_ptr<PowerPackItem> getPowerPack() { return powerPack; }
	/*
	Returns a reference to the music settings.
	*/
	inline MusicSettings& getMusicSettings() { return musicSettings; }
	inline std::string getBackgroundFileName() { return backgroundFileName; }
	inline float getBackgroundScrollSpeedX() { return backgroundScrollSpeedX; }
	inline float getBackgroundScrollSpeedY() { return backgroundScrollSpeedY; }
	inline sf::Color getBossNameColor() { return bossNameColor; }
	inline sf::Color getBossHPBarColor() { return bossHPBarColor; }

	inline void setName(std::string name) { this->name = name; }
	inline void setHealthPack(std::shared_ptr<HealthPackItem> healthPack) { this->healthPack = healthPack; }
	inline void setPointsPack(std::shared_ptr<PointsPackItem> pointPack) { this->pointPack = pointPack; }
	inline void setPowerPack(std::shared_ptr<PowerPackItem> powerPack) { this->powerPack = powerPack; }
	inline void setBackgroundFileName(std::string backgroundFileName) { this->backgroundFileName = backgroundFileName; }
	inline void setBackgroundScrollSpeedX(float backgroundScrollSpeedX) { this->backgroundScrollSpeedX = backgroundScrollSpeedX; }
	inline void setBackgroundScrollSpeedY(float backgroundScrollSpeedY) { this->backgroundScrollSpeedY = backgroundScrollSpeedY; }
	inline void setBossNameColor(sf::Color bossNameColor) { this->bossNameColor = bossNameColor; }
	inline void setBossHPBarColor(sf::Color bossHPBarColor) { this->bossHPBarColor = bossHPBarColor; }

	// Inserts a spawn condition and enemies such that the new condition and enemies are at the specified index
	inline void insertEnemySpawns(int conditionIndex, std::shared_ptr<EnemySpawnCondition> spawnCondition, std::vector<EnemySpawnInfo> enemies) {
		enemyGroups.insert(enemyGroups.begin() + conditionIndex, std::make_pair(spawnCondition, enemies)); 
	}
	// Adds an enemy into an already existing spawn condition
	inline void addEnemy(int conditionIndex, EnemySpawnInfo enemy) {
		enemyGroups[conditionIndex].second.push_back(enemy);
	}

	inline bool conditionSatisfied(int conditionIndex, entt::DefaultRegistry& registry) { return enemyGroups[conditionIndex].first->satisfied(registry); }

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

	// Music to play on start of level
	MusicSettings musicSettings;

	std::string backgroundFileName;
	float backgroundScrollSpeedX = 0;
	float backgroundScrollSpeedY = 0;

	// Boss name and hp bar colors are in Level instead of in Enemy because they are drawn in the playing area, 
	// on top of the background, so they should be up to Level to decide
	sf::Color bossNameColor = sf::Color::White;
	sf::Color bossHPBarColor = sf::Color::Red;
};