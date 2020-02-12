#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <SFML/Audio.hpp>
#include "MovablePoint.h"
#include "SpriteLoader.h"
#include "TextMarshallable.h"
#include "Components.h"
#include "AudioPlayer.h"
#include <entt/entt.hpp>

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

	std::shared_ptr<EditorPlayer> getPlayer();
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

	void load();
	void save();

	void deleteTemporaryEditorObjecs();

	/*
	Creates the sprite loader that contains info for all animatables that are used in this level pack.
	*/
	std::unique_ptr<SpriteLoader> createSpriteLoader();

	void insertLevel(int index, std::shared_ptr<Level> level);
	std::shared_ptr<EditorAttack> createAttack(bool addAttackToLevelPack = true);
	/*
	Updates an attack.
	If the attack ID is already in the LevelPack, overwrite the attack.
	If the attack ID is not in the LevelPack, add in the attack.
	*/
	void updateAttack(std::shared_ptr<EditorAttack> attack);
	std::shared_ptr<EditorAttackPattern> createAttackPattern();
	std::shared_ptr<EditorEnemy> createEnemy();
	std::shared_ptr<EditorEnemyPhase> createEnemyPhase();
	std::shared_ptr<BulletModel> createBulletModel();

	/*
	Creates a temporary EditorAttack that will not be saved.
	*/
	std::shared_ptr<EditorAttack> createTempAttack();
	/*
	Creates a temporary EditorAttackPattern that will not be saved.
	*/
	std::shared_ptr<EditorAttackPattern> createTempAttackPattern();
	/*
	Creates a temporary EditorEnemy that will not be saved.
	*/
	std::shared_ptr<EditorEnemy> createTempEnemy();
	/*
	Creates a temporary EditorEnemyPhase that will not be saved.
	*/
	std::shared_ptr<EditorEnemyPhase> createTempEnemyPhase();

	void deleteLevel(int levelIndex);
	void deleteAttack(int id);
	void deleteAttackPattern(int id);
	void deleteEnemy(int id);
	void deleteEnemyPhase(int id);

	/*
	Returns a list of indices of Levels that use the EditorEnemy
	with ID enemyID.
	*/
	std::vector<int>& getEnemyUsers(int enemyID);
	/*
	Returns a list of IDs of EditorEnemies that use the EditorEnemyPhase
	with ID editorEnemyPhaseID.
	*/
	std::vector<int>& getEditorEnemyUsers(int editorEnemyPhaseID);
	/*
	Returns a list of IDs of Players that use the EditorAttackPattern
	with ID attackPatternID.

	TODO: make Player objects have IDs and stuff
	*/
	std::vector<int>& getAttackPatternPlayerUsers(int attackPatternID);
	/*
	Returns a list of IDs of EditorEnemyPhases that use the EditorAttackPattern
	with ID attackPatternID.
	*/
	std::vector<int>& getAttackPatternEnemyUsers(int attackPatternID);
	/*
	Returns a list of IDs of EditorAttackPatterns that use the EditorAttack
	with ID attackID.
	*/
	std::vector<int>& getAttackUsers(int attackID);
	/*
	Returns a list of IDs of EditorAttacks that use the BulletModel
	with ID bulletModelID.
	*/
	std::vector<int>& getBulletModelUsers(int bulletModelID);

	bool hasEnemy(int id);
	bool hasEnemyPhase(int id);
	bool hasAttackPattern(int id);
	bool hasAttack(int id);
	bool hasBulletModel(int id);
	bool hasLevel(int levelIndex);

	std::string getName();
	std::shared_ptr<Level> getLevel(int levelIndex) const;
	std::shared_ptr<const EditorAttack> getAttack(int id) const;
	std::shared_ptr<EditorAttackPattern> getAttackPattern(int id) const;
	std::shared_ptr<EditorEnemy> getEnemy(int id) const;
	std::shared_ptr<EditorEnemyPhase> getEnemyPhase(int id) const;
	std::shared_ptr<BulletModel> getBulletModel(int id) const;
	std::shared_ptr<EditorPlayer> getPlayer();
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

	int getNextAttackID() const { return nextAttackID; }
	int getNextAttackPatternID() const { return nextAttackPatternID; }
	int getNextEnemyID() const { return nextEnemyID; }
	int getNextEnemyPhaseID() const { return nextEnemyPhaseID; }
	int getNextBulletModelID() const { return nextBulletModelID; }

	std::shared_ptr<entt::SigH<void()>> getOnChange();

	bool hasBulletModel(int id) const;

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

	int nextAttackID = 0;
	int nextAttackPatternID = 0;
	int nextEnemyID = 0;
	int nextEnemyPhaseID = 0;
	int nextBulletModelID = 0;

	int nextTempAttackID = -1;
	int nextTempAttackPatternID = -1;
	int nextTempEnemyID = -1;
	int nextTempEnemyPhaseID = -1;

	// ordered levels
	std::vector<std::shared_ptr<Level>> levels;
	// The IDs of EditorAttack/EditorAttackPattern/EditorEnemy/EditorEnemyPhase are always positive, unless it is a temporary object.
	// Temporary Editor_____ objects are deleted when deleteTemporaryEditorObjects() is called, and they cannot be saved in save().
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

	// Called when a change is made to the level pack
	std::shared_ptr<entt::SigH<void()>> onChange;
};