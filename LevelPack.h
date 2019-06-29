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
#include "AudioPlayer.h"
#include "Player.h"
#include "Level.h"

// REMEMBER: THESE CLASSES ARE NEVER MODIFIED
// there will be Components that will keep track of deltaTimes
// to actually carry out the concrete stuff similar to with MPs

/*
Attacks, attack patterns, enemies, and enemy phases can be saved by name.
*/

class LevelPackMetadata : public TextMarshallable {
public:
	std::string format() override;
	void load(std::string formattedString) override;

	inline EditorPlayer getPlayer() { return player; }
	inline const std::vector<std::pair<std::string, std::string>>& getSpriteSheets() { return spriteSheets; }

	inline void setPlayer(EditorPlayer player) { this->player = player; }

	inline void addSpriteSheet(std::string spriteSheetMetadataFileName, std::string spriteSheetImageFileName) {
		spriteSheets.push_back(std::make_pair(spriteSheetMetadataFileName, spriteSheetImageFileName));
	}
	inline void removeSpriteSheet(int index) {
		spriteSheets.erase(spriteSheets.begin() + index);
	}

private:
	EditorPlayer player;
	// Maps sprite sheet metadata file names to sprite sheet image file names
	std::vector<std::pair<std::string, std::string>> spriteSheets;
};

class LevelPack {
public:
	LevelPack(AudioPlayer& audioPlayer, std::string name);

	void load();
	void save();

	/*
	Creates the sprite loader that contains info for all animatables that are used in this level pack.
	*/
	std::unique_ptr<SpriteLoader> createSpriteLoader();

	inline void insertLevel(int index, std::shared_ptr<Level> level) {
		levels.insert(levels.begin() + index, level);
	}
	inline std::shared_ptr<EditorAttack> createAttack(bool addAttackToLevelPack = true) {
		auto attack = std::make_shared<EditorAttack>(nextAttackID++);
		if (addAttackToLevelPack) {
			attacks[attack->getID()] = attack;
		}
		return attack;
	}
	/*
	Updates an attack.
	If the attack ID is already in the LevelPack, overwrite the attack.
	If the attack ID is not in the LevelPack, add in the attack.
	*/
	inline void updateAttack(std::shared_ptr<EditorAttack> attack) {
		attacks[attack->getID()] = attack;
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
	inline std::shared_ptr<BulletModel> createBulletModel() {
		auto bulletModel = std::make_shared<BulletModel>(nextBulletModelID++);
		bulletModels[bulletModel->getID()] = bulletModel;
		return bulletModel;
	}

	inline void deleteLevel(int levelIndex) { levels.erase(levels.begin() + levelIndex); }
	inline void deleteAttack(int id) { attacks.erase(id); }
	inline void deleteAttackPattern(int id) { attackPatterns.erase(id); }
	inline void deleteEnemy(int id) { enemies.erase(id); }
	inline void deleteEnemyPhase(int id) { enemyPhases.erase(id); }

	inline std::string getName() { return name; }
	inline std::shared_ptr<Level> getLevel(int levelIndex) const { return levels[levelIndex]; }
	inline std::shared_ptr<EditorAttack> getAttack(int id) const { return attacks.at(id); }
	inline std::shared_ptr<EditorAttackPattern> getAttackPattern(int id) const { return attackPatterns.at(id); }
	inline std::shared_ptr<EditorEnemy> getEnemy(int id) const { return enemies.at(id); }
	inline std::shared_ptr<EditorEnemyPhase> getEnemyPhase(int id) const { return enemyPhases.at(id); }
	inline std::shared_ptr<BulletModel> getBulletModel(int id) const { return bulletModels.at(id); }
	inline EditorPlayer getPlayer() { return metadata.getPlayer(); }
	inline std::string getFontFileName() { return fontFileName; }

	inline std::map<int, std::shared_ptr<EditorAttack>>::iterator getAttackIteratorBegin() { return attacks.begin(); }
	inline std::map<int, std::shared_ptr<EditorAttack>>::iterator getAttackIteratorEnd() { return attacks.end(); }
	inline std::map<int, std::shared_ptr<EditorAttackPattern>>::iterator getAttackPatternIteratorBegin() { return attackPatterns.begin(); }
	inline std::map<int, std::shared_ptr<EditorAttackPattern>>::iterator getAttackPatternIteratorEnd() { return attackPatterns.end(); }
	inline std::map<int, std::shared_ptr<EditorEnemy>>::iterator getEnemyIteratorBegin() { return enemies.begin(); }
	inline std::map<int, std::shared_ptr<EditorEnemy>>::iterator getEnemyIteratorEnd() { return enemies.end(); }
	inline std::map<int, std::shared_ptr<EditorEnemyPhase>>::iterator getEnemyPhaseIteratorBegin() { return enemyPhases.begin(); }
	inline std::map<int, std::shared_ptr<EditorEnemyPhase>>::iterator getEnemyPhaseIteratorEnd() { return enemyPhases.end(); }
	inline std::map<int, std::shared_ptr<BulletModel>>::iterator getBulletModelIteratorBegin() { return bulletModels.begin(); }
	inline std::map<int, std::shared_ptr<BulletModel>>::iterator getBulletModelIteratorEnd() { return bulletModels.end(); }

	inline int getNextAttackID() const { return nextAttackID; }
	inline int getNextAttackPatternID() const { return nextAttackPatternID; }
	inline int getNextEnemyID() const { return nextEnemyID; }
	inline int getNextEnemyPhaseID() const { return nextEnemyPhaseID; }
	inline int getNextBulletModelID() const { return nextBulletModelID; }

	inline bool hasBulletModel(int id) const { return bulletModels.count(id) > 0; }

	inline void setPlayer(EditorPlayer player) { metadata.setPlayer(player); }
	inline void setFontFileName(std::string fontFileName) { this->fontFileName = fontFileName; }

	// Returns the radius of the largest bullet in the level pack
	float searchLargestBulletHitbox() const;
	// Returns the radius of the largest item activation radius in the level pack
	float searchLargestItemActivationHitbox() const;
	// Returns the radius of the largest item hitbox radius in the level pack
	float searchLargestItemCollectionHitbox() const;

	void playSound(const SoundSettings& soundSettings) const;
	void playMusic(const MusicSettings& musicSettings) const;

private:
	std::string name;

	AudioPlayer& audioPlayer;

	LevelPackMetadata metadata;

	int nextAttackID = 0;
	int nextAttackPatternID = 0;
	int nextEnemyID = 0;
	int nextEnemyPhaseID = 0;
	int nextBulletModelID = 0;

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
	// bullet model id : bullet model
	std::map<int, std::shared_ptr<BulletModel>> bulletModels;

	std::string fontFileName = "font.ttf";
};