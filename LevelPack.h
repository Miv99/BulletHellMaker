#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <SFML/Audio.hpp>
#include "MovablePoint.h"
#include "EnemyPhaseAction.h"
#include "AttackPattern.h"
#include "Enemy.h"
#include "EnemyPhase.h"
#include "SpriteLoader.h"
#include "EditorMovablePointSpawnType.h"
#include "EditorMovablePoint.h"
#include "EditorMovablePointAction.h"
#include "EnemyPhaseAction.h"
#include "Player.h"
#include "TextMarshallable.h"
#include "Components.h"
#include "Attack.h"
#include "Level.h"

// REMEMBER: THESE CLASSES ARE NEVER MODIFIED
// there will be Components that will keep track of deltaTimes
// to actually carry out the concrete stuff similar to with MPs

/*
Attacks, attack patterns, enemies, and enemy phases can be saved by name.
*/

class LevelPack {
public:
	LevelPack(std::string name);

	void load();
	void save();

	void update();

	void preloadTextures();

	inline void insertLevel(int index, std::shared_ptr<Level> level) {
		levels.insert(levels.begin() + index, level);
	}
	inline std::shared_ptr<EditorAttack> createAttack() {
		auto attack = std::make_shared<EditorAttack>(nextAttackID++);
		attacks[attack->getID()] = attack;
		return attack;
	}
	inline std::shared_ptr<EditorAttackPattern> createAttackPattern() {
		auto attackPattern = std::make_shared<EditorAttackPattern>(nextAttackPatternID++);
		attackPatterns[attackPattern->getID()] = attackPattern;
		return attackPattern; 
	}
	inline std::shared_ptr<EditorEnemy> createEnemy() {
		auto enemy = std::make_shared<EditorEnemy>(nextEnemyID++);
		enemies[enemy->getID()] = enemy;
		return enemy; 
	}
	inline std::shared_ptr<EditorEnemyPhase> createEnemyPhase() {
		auto enemyPhase = std::make_shared<EditorEnemyPhase>(nextEnemyPhaseID++);
		enemyPhases[enemyPhase->getID()] = enemyPhase;
		return enemyPhase; 
	}

	inline void deleteLevel(int levelIndex) { levels.erase(levels.begin() + levelIndex); }
	inline void deleteAttack(int id) { attacks.erase(id); }
	inline void deleteAttackPattern(int id) { attackPatterns.erase(id); }
	inline void deleteEnemy(int id) { enemies.erase(id); }
	inline void deleteEnemyPhase(int id) { enemyPhases.erase(id); }

	inline std::shared_ptr<Level> getLevel(int levelIndex) const { return levels[levelIndex]; }
	inline std::shared_ptr<EditorAttack> getAttack(int id) const  { return attacks.at(id); }
	inline std::shared_ptr<EditorAttackPattern> getAttackPattern(int id) const  { return attackPatterns.at(id); }
	inline std::shared_ptr<EditorEnemy> getEnemy(int id) const  { return enemies.at(id); }
	inline std::shared_ptr<EditorEnemyPhase> getEnemyPhase(int id) const { return enemyPhases.at(id); }

	float searchLargestHitbox();

	void playSound(std::string fileName, float volume);

private:
	std::string name;

	std::unique_ptr<SpriteLoader> spriteLoader;

	int nextAttackID = 0;
	int nextAttackPatternID = 0;
	int nextEnemyID = 0;
	int nextEnemyPhaseID = 0;

	// ordered levels
	std::vector<std::shared_ptr<Level>> levels;
	// attack id : attack
	std::map<int, std::shared_ptr<EditorAttack>> attacks;
	// attack pattern id : attack pattern
	std::map<int, std::shared_ptr<EditorAttackPattern>> attackPatterns;
	// enemy id : enemy
	std::map<int, std::shared_ptr<EditorEnemy>> enemies;
	// enemy phase id : enemy phase
	std::map<int, std::shared_ptr<EditorEnemyPhase>> enemyPhases;

	// Maps file names to SoundBuffers. Not saved.
	std::map<std::string, sf::SoundBuffer> soundBuffers;
	// Queue of sounds currently playing. Not saved.
	std::queue<std::unique_ptr<sf::Sound>> currentSounds;
};