#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>

#include <SFML/Audio.hpp>
#include <entt/entt.hpp>

#include <exprtk.hpp>
#include <DataStructs/SpriteLoader.h>
#include <DataStructs/IDGenerator.h>
#include <LevelPack/TextMarshallable.h>
#include <Game/AudioPlayer.h>

class LayerRootLevelPackObject;
class LevelPackObject;
class Level;
class EditorAttack;
class EditorAttackPattern;
class EditorMovablePoint;
class EditorEnemy;
class EditorEnemyPhase;
class EditorPlayer;
class BulletModel;

class LevelPack {
public:
	/*
	The list of types of LevelPackObjects that can be the roots of a layer in the
	LevelPackObject hierarchy. LevelPackObjects of these types can reference each other
	only by ID.
	*/
	enum class LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE {
		SPRITE_SHEET,
		PLAYER,
		LEVEL,
		ENEMY,
		ENEMY_PHASE,
		ATTACK_PATTERN,
		ATTACK,
		BULLET_MODEL
	};
	struct LoadMetrics {
		bool playerSuccess = false;

		int levelsFailed = 0;
		int levelsTotal = 0;

		int enemiesFailed = 0;
		int enemiesTotal = 0;

		int enemyPhasesFailed = 0;
		int enemyPhasesTotal = 0;

		int attackPatternsFailed = 0;
		int attackPatternsTotal = 0;

		int attacksFailed = 0;
		int attacksTotal = 0;

		int bulletModelsFailed = 0;
		int bulletModelsTotal = 0;

		std::string formatForUser();
		bool containsFailedLoads();
	};

	/*
	spriteLoader - if not nullptr, this sprite loader will be used in the newly loaded level pack
		so that textures don't have to be loaded twice
	*/
	LevelPack(AudioPlayer& audioPlayer, std::string name, std::shared_ptr<SpriteLoader> spriteLoader = nullptr);

	/*
	Load the LevelPack from its folder.
	*/
	LoadMetrics load();
	/*
	Save the LevelPack into its folder.
	*/
	void save();

	/*
	Insert a Level's ID into this LevelPack at the specified index.
	*/
	void insertLevel(int index, int levelID);

	/*
	Create a Level without adding it to the levels list order.
	*/
	std::shared_ptr<Level> createLevel();
	/*
	Create a Level and add it to this LevelPack.
	id - the ID of the new level. If it is already in use, the old Level
		with this ID will be overwritten.
	*/
	std::shared_ptr<Level> createLevel(int id);
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
	Create an EditorAttackPattern and add it to this LevelPack.
	id - the ID of the new attack pattern. If it is already in use, the old EditorAttackPattern
		with this ID will be overwritten.
	*/
	std::shared_ptr<EditorAttackPattern> createAttackPattern(int id);
	/*
	Create an EditorEnemy and add it to this LevelPack.
	*/
	std::shared_ptr<EditorEnemy> createEnemy();
	/*
	Create an EditorEnemy and add it to this LevelPack.
	id - the ID of the new enemy. If it is already in use, the old EditorEnemy
		with this ID will be overwritten.
	*/
	std::shared_ptr<EditorEnemy> createEnemy(int id);
	/*
	Create an EditorEnemyPhase and add it to this LevelPack.
	*/
	std::shared_ptr<EditorEnemyPhase> createEnemyPhase();
	/*
	Create an EditorEnemyPhase and add it to this LevelPack.
	id - the ID of the new enemy phase. If it is already in use, the old EditorEnemyPhase
		with this ID will be overwritten.
	*/
	std::shared_ptr<EditorEnemyPhase> createEnemyPhase(int id);
	/*
	Create an BulletModel and add it to this LevelPack.
	*/
	std::shared_ptr<BulletModel> createBulletModel();

	/*
	Updates an sprite sheet.
	If the sprite sheet name is already in the LevelPack's SpriteLoader, overwrite the sprite sheet.
	If the sprite sheet name is not in the LevelPack's SpriteLoader, add in the sprite sheet.
	*/
	void updateSpriteSheet(std::shared_ptr<SpriteSheet> spriteSheet, bool emitOnChange = true);
	/*
	Updates an attack.
	If the attack ID is already in the LevelPack, overwrite the attack.
	If the attack ID is not in the LevelPack, add in the attack.
	*/
	void updateAttack(std::shared_ptr<EditorAttack> attack, bool emitOnChange = true);
	/*
	Updates an attack.
	If the attack ID is already in the LevelPack, overwrite the attack.
	If the attack ID is not in the LevelPack, add in the attack.
	*/
	void updateAttack(std::shared_ptr<LayerRootLevelPackObject> attack, bool emitOnChange = true);
	/*
	Updates an attack pattern.
	If the attack pattern ID is already in the LevelPack, overwrite the attack pattern.
	If the attack pattern ID is not in the LevelPack, add in the attack pattern.
	*/
	void updateAttackPattern(std::shared_ptr<EditorAttackPattern> attackPattern, bool emitOnChange = true);
	/*
	Updates an attack pattern.
	If the attack pattern ID is already in the LevelPack, overwrite the attack pattern.
	If the attack pattern ID is not in the LevelPack, add in the attack pattern.
	*/
	void updateAttackPattern(std::shared_ptr<LayerRootLevelPackObject> attackPattern, bool emitOnChange = true);
	/*
	Updates an enemy.
	If the enemy ID is already in the LevelPack, overwrite the enemy.
	If the enemy ID is not in the LevelPack, add in the enemy.
	*/
	void updateEnemy(std::shared_ptr<EditorEnemy> enemy, bool emitOnChange = true);
	/*
	Updates an enemy.
	If the enemy ID is already in the LevelPack, overwrite the enemy.
	If the enemy ID is not in the LevelPack, add in the enemy.
	*/
	void updateEnemy(std::shared_ptr<LayerRootLevelPackObject> enemy, bool emitOnChange = true);
	/*
	Updates an enemy phase.
	If the enemy phase ID is already in the LevelPack, overwrite the enemy phase.
	If the enemy phase ID is not in the LevelPack, add in the enemy phase.
	*/
	void updateEnemyPhase(std::shared_ptr<EditorEnemyPhase> enemyPhase, bool emitOnChange = true);
	/*
	Updates an enemy phase.
	If the enemy phase ID is already in the LevelPack, overwrite the enemy phase.
	If the enemy phase ID is not in the LevelPack, add in the enemy phase.
	*/
	void updateEnemyPhase(std::shared_ptr<LayerRootLevelPackObject> enemyPhase, bool emitOnChange = true);
	/*
	Updates a bullet model.
	If the bullet model ID is already in the LevelPack, overwrite the bullet model.
	If the bullet model ID is not in the LevelPack, add in the bullet model.
	*/
	void updateBulletModel(std::shared_ptr<BulletModel> bulletModel, bool emitOnChange = true);
	/*
	Updates a bullet model.
	If the bullet model ID is already in the LevelPack, overwrite the bullet model.
	If the bullet model ID is not in the LevelPack, add in the bullet model.
	*/
	void updateBulletModel(std::shared_ptr<LayerRootLevelPackObject> bulletModel, bool emitOnChange = true);

	/*
	Removes a level from the list of levels that the player can play.
	*/
	void removeLevelFromPlayableLevelsList(int levelIndex);
	/*
	Deletes a level from this LevelPack. This also removes the level from
	the playable levels list if it was there.
	*/
	void deleteLevel(int id);
	void deleteAttack(int id);
	void deleteAttackPattern(int id);
	void deleteEnemy(int id);
	void deleteEnemyPhase(int id);
	void deleteBulletModel(int id);

	/*
	Returns the sprite loader that contains info for all animatables that are used in this level pack.
	*/
	std::shared_ptr<SpriteLoader> getSpriteLoader();

	/*
	Returns a list of IDs of Levels that use the EditorEnemy
	with ID enemyID.
	*/
	std::vector<int> getEnemyUsers(int enemyID);
	/*
	Returns a list of IDs of EditorEnemies that use the EditorEnemyPhase
	with ID editorEnemyPhaseID.
	*/
	std::vector<int> getEditorEnemyUsers(int editorEnemyPhaseID);
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

	/*
	Returns whether the attack pattern with ID attackPatternID is used
	by the player.
	*/
	bool getAttackPatternIsUsedByPlayer(int attackPatternID);

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

	/*
	Returns an EditorAttackPattern for editing purposes.
	*/
	std::shared_ptr<EditorAttackPattern> getAttackPattern(int id) const;
	/*
	Returns an EditorAttackPattern for gameplay purposes.
	*/
	std::shared_ptr<EditorAttackPattern> getGameplayAttackPattern(int id, exprtk::symbol_table<float> symbolsDefiner) const;
	/*
	Returns an EditorEnemy for editing purposes.
	*/
	std::shared_ptr<EditorEnemy> getEnemy(int id) const;
	/*
	Returns an EditorEnemy for gameplay purposes.

	symbolsDefiner - a symbol_table that defines all symbols redelegated in the EditorEnemy's ValueSymbolTable
	*/
	std::shared_ptr<EditorEnemy> getGameplayEnemy(int id, exprtk::symbol_table<float> symbolsDefiner) const;

	/*
	Returns an EditorEnemyPhase for editing purposes.
	*/
	std::shared_ptr<EditorEnemyPhase> getEnemyPhase(int id) const;
	/*
	Returns an EditorEnemyPhase for gameplay purposes.
	*/
	std::shared_ptr<EditorEnemyPhase> getGameplayEnemyPhase(int id, exprtk::symbol_table<float> symbolsDefiner) const;

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

	/*
	Returns whether this LevelPack attempted to load from a folder.
	*/
	bool getAttemptedLoad() const;
	/*
	Returns whether this LevelPack successfully loaded from a folder.
	*/
	bool getSuccessfulLoad() const;

	std::shared_ptr<entt::SigH<void(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE, int)>> getOnChange();

	void setPlayer(std::shared_ptr<EditorPlayer> player);
	void setFontFileName(std::string fontFileName) { this->fontFileName = fontFileName; }

	// Returns the radius of the largest bullet in the level pack
	float searchLargestBulletHitbox() const;
	// Returns the radius of the largest item activation radius in the level pack
	float searchLargestItemActivationHitbox() const;
	// Returns the radius of the largest item hitbox radius in the level pack
	float searchLargestItemCollectionHitbox() const;

	void playSound(const SoundSettings& soundSettings) const;
	/*
	Returns the Music object that is played from this function call.

	See AudioPlayer::playMusic() for more info.
	*/
	std::shared_ptr<sf::Music> playMusic(const MusicSettings& musicSettings) const;
	/*
	See AudioPlayer::playMusic() for more info.
	*/
	void playMusic(std::shared_ptr<sf::Music> music, const MusicSettings& musicSettings) const;

private:
	const static std::string LEVELS_ORDER_FILE_NAME;
	const static std::string PLAYER_FILE_NAME;

	const static std::string LEVEL_FILE_PREFIX;
	const static std::string ATTACK_FILE_PREFIX;
	const static std::string ATTACK_PATTERN_FILE_PREFIX;
	const static std::string BULLET_MODEL_FILE_PREFIX;
	const static std::string ENEMY_FILE_PREFIX;
	const static std::string ENEMY_PHASE_FILE_PREFIX;

	// Whether this LevelPack was attempted to be loaded
	bool attemptedLoad = false;
	// Whether this LevelPack was loaded successfully
	bool successfulLoad;

	std::string name;

	std::shared_ptr<SpriteLoader> spriteLoader;
	AudioPlayer& audioPlayer;

	IDGenerator levelIDGen;
	IDGenerator attackIDGen;
	IDGenerator attackPatternIDGen;
	IDGenerator enemyIDGen;
	IDGenerator enemyPhaseIDGen;
	IDGenerator bulletModelIDGen;

	std::shared_ptr<EditorPlayer> player;
	// Ordered level IDs
	std::vector<int> levels;
	// The IDs of Level/EditorAttack/EditorAttackPattern/EditorEnemy/EditorEnemyPhase are always positive
	// Maps level ID to the level
	std::map<int, std::shared_ptr<Level>> levelsMap;
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

	/*
	Emitted right after whenever a change is made to an object of a type included in
	LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE.

	Parameters:
		LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE - the type of the LevelPackObject that was just modified
		int - the ID of the LevelPackObject

	*/
	std::shared_ptr<entt::SigH<void(LEVEL_PACK_OBJECT_HIERARCHY_LAYER_ROOT_TYPE, int)>> onChange;

	/*
	Returns a set of IDs of level pack object files currently existing in a folder.
	Level pack object files are assumed to be in the format "[levelPackObjectFilePrefix][ID][levelPackObjectFileExtension]"
	ie "level0.xml"

	folderPath - the path of the folder relative to the game root
	levelPackObjectFilePrefix - a file in folderPath must start with levelPackObjectFilePrefix to be counted as a level pack object file
	levelPackObjectFileExtension - a file in folderPath must have this extension to be counted as a level pack object file (ie ".xml")
	*/
	std::set<int> getAllExistingLevelPackObjectFilesIDs(std::string folderPath, std::string levelPackObjectFilePrefix, std::string levelPackObjectFileExtension);
	/*
	Deletes all level pack object files in the folderPath folder that have an ID in ids.
	Failed deletions are logged as warnings.

	levelPackObjectFilePrefix - a file in folderPath must start with levelPackObjectFilePrefix to be counted as a level pack object file
	levelPackObjectFileExtension - a file in folderPath must have this extension to be counted as a level pack object file (ie ".xml")
	ids - the set of IDs of level pack objects to be deleted
	*/
	void deleteLevelPackObjectFiles(std::string folderPath, std::string levelPackObjectFilePrefix, std::string levelPackObjectFileExtension, std::set<int> ids);
};