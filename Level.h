#pragma once
#include <vector>
#include <utility>
#include <string>
#include <entt/entt.hpp>
#include <memory>
#include "EnemySpawnCondition.h"
#include "EnemySpawn.h"
#include "TextMarshallable.h"

class Level : public TextMarshallable {
public:
	inline Level() {}
	inline Level(std::string name) : name(name) {}

	std::string format() override;
	void load(std::string formattedString) override;

	inline std::string getName() { return name; }
	inline const std::vector<EnemySpawnInfo>& getEnemyGroupSpawnInfo(int conditionIndex) { return enemyGroups[conditionIndex].second; }
	inline int getEnemyGroupsCount() { return enemyGroups.size(); }
	inline void setName(std::string name) { this->name = name; }

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
	std::vector<std::pair<std::shared_ptr<EnemySpawnCondition>, std::vector<EnemySpawnInfo>>> enemyGroups;
};