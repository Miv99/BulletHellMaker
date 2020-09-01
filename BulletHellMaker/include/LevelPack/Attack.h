#pragma once
#include <string>
#include <vector>
#include <utility>
#include <algorithm>

#include <entt/entt.hpp>

#include <DataStructs/SpriteLoader.h>
#include <LevelPack/TextMarshallable.h>
#include <Game/EntityCreationQueue.h>
#include <LevelPack/LevelPackObject.h>
#include <DataStructs/IDGenerator.h>

class EditorMovablePoint;

class EditorAttack : public LevelPackObject {
public:
	EditorAttack();
	EditorAttack(int id);
	/*
	Copy constructor.
	*/
	EditorAttack(std::shared_ptr<const EditorAttack> copy);
	/*
	Copy constructor.
	*/
	EditorAttack(const EditorAttack* copy);

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	void loadEMPBulletModels(const LevelPack& levelPack);

	/*
	Executes the attack as an enemy.

	entity - the enemy that is executing this attack
	timeLag - the time elapsed since this attack was supposed to execute
	attackPatternID - the ID of the enemy's current attack pattern
	enemyID - the ID of the enemy executing this attack
	enemyPhaseID - the ID of the enemy's current phase 
	*/
	void executeAsEnemy(EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry& registry, uint32_t entity, float timeLag, int attackPatternID, int enemyID, int enemyPhaseID) const;
	/*
	Executes the attack as a player.

	entity - the enemy that is executing this attack
	timeLag - the time elapsed since this attack was supposed to execute
	attackPatternID - the ID of the enemy's current attack pattern
	*/
	void executeAsPlayer(EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry& registry, uint32_t entity, float timeLag, int attackPatternID) const;

	/*
	Returns whether this EditorAttack and any of its its children EMPs uses the BulletModel
	with ID bulletModelID.
	*/
	const bool usesBulletModel(int bulletModelID) const;
	bool getPlayAttackAnimation() const;
	std::shared_ptr<EditorMovablePoint> getMainEMP() const;
	IDGenerator* getNextEMPID();
	std::map<int, int>* getBulletModelIDsCount();
	void setPlayAttackAnimation(bool playAttackAnimation);
	
	float searchLargestHitbox() const;
	// Search for the EMP with the ID
	std::shared_ptr<EditorMovablePoint> searchEMP(int id) const;

	/*
	Generates a list of string vectors such that, when each all the string vectors are added to a tgui::TreeView,
	the tree hierarchy of this EditorAttack, including all its EMPs, is created. Each entry is an EMP's ID.

	attackText - a function that takes an EditorAttack and returns a string -- the text in the tgui::TreeView for that EditorAttack
	empText - a function that takes an EMP and returns a string -- the text in the tgui::TreeView for that EMP
	*/
	std::vector<std::vector<sf::String>> generateTreeViewHierarchy(std::function<sf::String(const EditorAttack&)> attackText, std::function<sf::String(const EditorMovablePoint&)> empText) const;

	/*
	For testing.
	*/
	bool operator==(const EditorAttack& other) const;

private:
	// The ID generator for EMPs in this attack
	IDGenerator empIDGen;

	// Root of the EMP tree (the main EMP); always has ID 0
	// When the attack is executed, the mainEMP is spawned instantly. Its spawn type's time is ignored.
	std::shared_ptr<EditorMovablePoint> mainEMP;
	// Whether or not to play the enemy's attack animation with this attack
	bool playAttackAnimation = true;

	// Map that maps the bullet model ids used by this EMP and all children EMP to the number of times
	// that bullet model id is used.
	// This map will not be saved when format() is called, but will be rebuilt in load().
	std::map<int, int> bulletModelsCount;
};