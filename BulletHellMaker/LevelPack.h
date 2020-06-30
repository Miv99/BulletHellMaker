#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <SFML/Audio.hpp>
#include <entt/entt.hpp>
#include "SpriteLoader.h"
#include "TextMarshallable.h"
#include "AudioPlayer.h"
#include "IDGenerator.h"
#include "exprtk.hpp"

class LevelPackObject;
class Level;
class EditorAttack;
class EditorAttackPattern;
class EditorMovablePoint;
class EditorEnemy;
class EditorEnemyPhase;
class EditorPlayer;
class BulletModel;

class LevelPackMetadata : public TextMarshallable {
public:
	std::string format() const override;
	void load(std::string formattedString) override;

	std::shared_ptr<EditorPlayer> getPlayer() const;
	const std::vector<std::pair<std::string, std::string>>& getSpriteSheets() { return spriteSheets; }

	void setPlayer(std::shared_ptr<EditorPlayer> player);

	void addSpriteSheet(std::string spriteSheetMetadataFileName, std::string spriteSheetImageFileName) {
		spriteSheets.push_back(std::make_pair(spriteSheetMetadataFileName, spriteSheetImageFileName));
	}
	void removeSpriteSheet(int index) {
		spriteSheets.erase(spriteSheets.begin() + index);
	}

private:
	std::shared_ptr<EditorPlayer> player;
	// Pairs of sheet metadata file name and sprite sheet image file name, in that order
	std::vector<std::pair<std::string, std::string>> spriteSheets;
};

class LevelPack {
public:
	LevelPack(AudioPlayer& audioPlayer, std::string name);

	/*
	Load the LevelPack from its folder.
	*/
	void load();
	/*
	Save the LevelPack into its folder.
	*/
	void save();

	/*
	Creates the sprite loader that contains info for all animatables that are used in this level pack.
	*/
	std::unique_ptr<SpriteLoader> createSpriteLoader();

	/*
	Insert a Level into this LevelPack at the specified index.
	*/
	void insertLevel(int index, std::shared_ptr<Level> level);

	/*
	Create an EditorAttack and add it to this LevelPack.
	*/
	std::shared_ptr<EditorAttack> createAttack();
	/*
	Create an EditorAttack and add it to this LevelPack.
	id - the ID of the new attack. If it is already in use, the old EditorAttack
		with this ID will be overwritten.
	*/
	std::shared_ptr<EditorAttack> createAttack(int id);
	/*
	Create an EditorAttackPattern and add it to this LevelPack.
	*/
	std::shared_ptr<EditorAttackPattern> createAttackPattern();
	/*
	Create an EditorEnemy and add it to this LevelPack.
	*/
	std::shared_ptr<EditorEnemy> createEnemy();
	/*
	Create an EditorEnemyPhase and add it to this LevelPack.
	*/
	std::shared_ptr<EditorEnemyPhase> createEnemyPhase();
	/*
	Create an BulletModel and add it to this LevelPack.
	*/
	std::shared_ptr<BulletModel> createBulletModel();

	/*
	Updates an attack.
	If the attack ID is already in the LevelPack, overwrite the attack.
	If the attack ID is not in the LevelPack, add in the attack.
	*/
	void updateAttack(std::shared_ptr<EditorAttack> attack);
	/*
	Updates an attack.
	If the attack ID is already in the LevelPack, overwrite the attack.
	If the attack ID is not in the LevelPack, add in the attack.
	*/
	void updateAttack(std::shared_ptr<LevelPackObject> attack);
	/*
	Updates an attack pattern.
	If the attack pattern ID is already in the LevelPack, overwrite the attack pattern.
	If the attack pattern ID is not in the LevelPack, add in the attack pattern.
	*/
	void updateAttackPattern(std::shared_ptr<EditorAttackPattern> attackPattern);
	/*
	Updates an attack pattern.
	If the attack pattern ID is already in the LevelPack, overwrite the attack pattern.
	If the attack pattern ID is not in the LevelPack, add in the attack pattern.
	*/
	void updateAttackPattern(std::shared_ptr<LevelPackObject> attackPattern);
	/*
	Updates an enemy.
	If the enemy ID is already in the LevelPack, overwrite the enemy.
	If the enemy ID is not in the LevelPack, add in the enemy.
	*/
	void updateEnemy(std::shared_ptr<EditorEnemy> enemy);
	/*
	Updates an enemy.
	If the enemy ID is already in the LevelPack, overwrite the enemy.
	If the enemy ID is not in the LevelPack, add in the enemy.
	*/
	void updateEnemy(std::shared_ptr<LevelPackObject> enemy);
	/*
	Updates an enemy phase.
	If the enemy phase ID is already in the LevelPack, overwrite the enemy phase.
	If the enemy phase ID is not in the LevelPack, add in the enemy phase.
	*/
	void updateEnemyPhase(std::shared_ptr<EditorEnemyPhase> enemyPhase);
	/*
	Updates an enemy phase.
	If the enemy phase ID is already in the LevelPack, overwrite the enemy phase.
	If the enemy phase ID is not in the LevelPack, add in the enemy phase.
	*/
	void updateEnemyPhase(std::shared_ptr<LevelPackObject> enemyPhase);
	/*
	Updates a bullet model.
	If the bullet model ID is already in the LevelPack, overwrite the bullet model.
	If the bullet model ID is not in the LevelPack, add in the bullet model.
	*/
	void updateBulletModel(std::shared_ptr<BulletModel> bulletModel);
	/*
	Updates a bullet model.
	If the bullet model ID is already in the LevelPack, overwrite the bullet model.
	If the bullet model ID is not in the LevelPack, add in the bullet model.
	*/
	void updateBulletModel(std::shared_ptr<LevelPackObject> bulletModel);

	void deleteLevel(int levelIndex);
	void deleteAttack(int id);
	void deleteAttackPattern(int id);
	void deleteEnemy(int id);
	void deleteEnemyPhase(int id);
	void deleteBulletModel(int id);

	/*
	Returns a list of indices of Levels that use the EditorEnemy
	with ID enemyID.
	*/
	std::vector<int> getEnemyUsers(int enemyID);
	/*
	Returns a list of IDs of EditorEnemies that use the EditorEnemyPhase
	with ID editorEnemyPhaseID.
	*/
	std::vector<int> getEditorEnemyUsers(int editorEnemyPhaseID);
	/*
	Returns whether the LevelPack's EditorPlayer uses the EditorAttackPattern
	with ID attackPatternID.
	*/
	bool attackPatternIsUsedByPlayer(int attackPatternID);
	/*
	Returns a list of IDs of EditorEnemyPhases that use the EditorAttackPattern
	with ID attackPatternID.
	*/
	std::vector<int> getAttackPatternEnemyUsers(int attackPatternID);
	/*
	Returns a list of IDs of EditorAttackPatterns that use the EditorAttack
	with ID attackID.
	*/
	std::vector<int> getAttackUsers(int attackID);
	/*
	Returns a list of IDs of EditorAttacks that use the BulletModel
	with ID bulletModelID.
	*/
	std::vector<int> getBulletModelUsers(int bulletModelID);

	bool hasEnemy(int id);
	bool hasEnemyPhase(int id);
	bool hasAttackPattern(int id);
	bool hasAttack(int id);
	bool hasBulletModel(int id);
	bool hasLevel(int levelIndex);
	bool hasBulletModel(int id) const;

	std::string getName();

	/*
	Returns a Level for editing purposes.
	*/
	std::shared_ptr<Level> getLevel(int levelIndex) const;
	/*
	Returns a Level for gameplay purposes.
	*/
	std::shared_ptr<Level> getGameplayLevel(int levelIndex) const;

	/*
	Returns an EditorAttack for editing purposes.
	*/
	std::shared_ptr<EditorAttack> getAttack(int id) const;
	/*
	Returns an EditorAttack for gameplay purposes.

	symbolsDefiner - a symbol_table that defines all symbols redelegated in the EditorAttack's ValueSymbolTable
	*/
	std::shared_ptr<EditorAttack> getGameplayAttack(int id, exprtk::symbol_table<float> symbolsDefiner) const;

	std::shared_ptr<EditorAttackPattern> getAttackPattern(int id) const;
	/*
	Returns an EditorEnemy for editing purposes.
	*/
	std::shared_ptr<EditorEnemy> getEnemy(int id) const;
	/*
	Returns an EditorEnemy for gameplay purposes.

	symbolsDefiner - a symbol_table that defines all symbols redelegated in the EditorEnemy's ValueSymbolTable
	*/
	std::shared_ptr<EditorEnemy> getGameplayEnemy(int id, exprtk::symbol_table<float> symbolsDefiner) const;
	std::shared_ptr<EditorEnemyPhase> getEnemyPhase(int id) const;
	std::shared_ptr<BulletModel> getBulletModel(int id) const;
	/*
	Returns an EditorPlayer for editing purposes.
	*/
	std::shared_ptr<EditorPlayer> getPlayer() const;
	/*
	Returns an EditorPlayer for gameplay purposes.
	*/
	std::shared_ptr<EditorPlayer> getGameplayPlayer() const;
	std::string getFontFileName();

	std::map<int, std::shared_ptr<EditorAttack>>::iterator getAttackIteratorBegin();
	std::map<int, std::shared_ptr<EditorAttack>>::iterator getAttackIteratorEnd();
	std::map<int, std::shared_ptr<EditorAttackPattern>>::iterator getAttackPatternIteratorBegin();
	std::map<int, std::shared_ptr<EditorAttackPattern>>::iterator getAttackPatternIteratorEnd();
	std::map<int, std::shared_ptr<EditorEnemy>>::iterator getEnemyIteratorBegin();
	std::map<int, std::shared_ptr<EditorEnemy>>::iterator getEnemyIteratorEnd();
	std::map<int, std::shared_ptr<EditorEnemyPhase>>::iterator getEnemyPhaseIteratorBegin();
	std::map<int, std::shared_ptr<EditorEnemyPhase>>::iterator getEnemyPhaseIteratorEnd();
	std::map<int, std::shared_ptr<BulletModel>>::iterator getBulletModelIteratorBegin();
	std::map<int, std::shared_ptr<BulletModel>>::iterator getBulletModelIteratorEnd();

	/*
	Returns the next attack ID to be generated without using it.
	*/
	int getNextAttackID() const;
	/*
	Returns the next attack pattern ID to be generated without using it.
	*/
	int getNextAttackPatternID() const;
	/*
	Returns the next enemy ID to be generated without using it.
	*/
	int getNextEnemyID() const;
	/*
	Returns the next enemy phase ID to be generated without using it.
	*/
	int getNextEnemyPhaseID() const;
	/*
	Returns the next bullet model ID to be generated without using it.
	*/
	int getNextBulletModelID() const;

	/*
	Returns the next count number of attack IDs to be generated without using them.
	If less than count IDs can be generated, this will return as many IDs as possible.
	*/
	std::set<int> getNextAttackIDs(int count) const;
	/*
	Returns the next count number of attack pattern IDs to be generated without using them.
	If less than count IDs can be generated, this will return as many IDs as possible.
	*/
	std::set<int> getNextAttackPatternIDs(int count) const;
	/*
	Returns the next count number of enemy IDs to be generated without using them.
	If less than count IDs can be generated, this will return as many IDs as possible.
	*/
	std::set<int> getNextEnemyIDs(int count) const;
	/*
	Returns the next count number of enemy phase IDs to be generated without using them.
	If less than count IDs can be generated, this will return as many IDs as possible.
	*/
	std::set<int> getNextEnemyPhaseIDs(int count) const;
	/*
	Returns the next count number of bullet model IDs to be generated without using them.
	If less than count IDs can be generated, this will return as many IDs as possible.
	*/
	std::set<int> getNextBulletModelIDs(int count) const;

	std::shared_ptr<entt::SigH<void()>> getOnChange();

	void setPlayer(std::shared_ptr<EditorPlayer> player);
	void setFontFileName(std::string fontFileName) { this->fontFileName = fontFileName; }

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

	IDGenerator attackIDGen;
	IDGenerator attackPatternIDGen;
	IDGenerator enemyIDGen;
	IDGenerator enemyPhaseIDGen;
	IDGenerator bulletModelIDGen;

	// Ordered levels
	std::vector<std::shared_ptr<Level>> levels;
	// The IDs of EditorAttack/EditorAttackPattern/EditorEnemy/EditorEnemyPhase are always positive
	// Maps attack ID to the attack
	std::map<int, std::shared_ptr<EditorAttack>> attacks;
	// Maps attack pattern ID to the attack pattern
	std::map<int, std::shared_ptr<EditorAttackPattern>> attackPatterns;
	// Maps enemy ID to the enemy
	std::map<int, std::shared_ptr<EditorEnemy>> enemies;
	// Maps enemy phase ID to the enemy phase
	std::map<int, std::shared_ptr<EditorEnemyPhase>> enemyPhases;
	// Maps bullet model ID to the bullet model
	std::map<int, std::shared_ptr<BulletModel>> bulletModels;

	std::string fontFileName;

	// Called when a change is made to one of the level pack objects, which
	// includes EditorAttack, EditorAttackPattern, EditorEnemy, EditorEnemyPhase,
	// Level, BulletModel, and EditorPlayer.
	std::shared_ptr<entt::SigH<void()>> onChange;
};