#pragma once
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <entt/entt.hpp>
#include "SpriteLoader.h"
#include "TextMarshallable.h"
#include "EntityCreationQueue.h"

class EditorMovablePoint;

class EditorAttack : public TextMarshallable {
public:
	inline EditorAttack() {}
	EditorAttack(int id);
	// Copy constructor
	EditorAttack(std::shared_ptr<const EditorAttack> copy);

	std::string format() const override;
	void load(std::string formattedString) override;

	bool legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::string& message) const;

	void loadEMPBulletModels(const LevelPack& levelPack);

	/*
	Executes the attack as an enemy.

	entity - the enemy that is executing this attack
	timeLag - the time elapsed since this attack was supposed to execute
	attackPatternID - the ID of the enemy's current attack pattern
	enemyID - the ID of the enemy executing this attack
	enemyPhaseID - the ID of the enemy's current phase 
	*/
	void EditorAttack::executeAsEnemy(EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry& registry, uint32_t entity, float timeLag, int attackPatternID, int enemyID, int enemyPhaseID) const;
	/*
	Executes the attack as a player.

	entity - the enemy that is executing this attack
	timeLag - the time elapsed since this attack was supposed to execute
	attackPatternID - the ID of the enemy's current attack pattern
	*/
	void EditorAttack::executeAsPlayer(EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry& registry, uint32_t entity, float timeLag, int attackPatternID) const;

	inline int getID() const { return id; }
	inline std::string getName() const { return name; }
	inline bool getPlayAttackAnimation() const { return playAttackAnimation; }
	inline std::shared_ptr<EditorMovablePoint> getMainEMP() const { return mainEMP; }
	inline void setName(std::string name)  { this->name = name; }
	inline void setPlayAttackAnimation(bool playAttackAnimation) { this->playAttackAnimation = playAttackAnimation; }
	
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

private:
	// The ID of the next EMP created in this attack.
	int nextEMPID = 0;

	// ID of the attack
	int id;
	// User-defined name of the attack
	std::string name;
	// Root of the EMP tree (the main EMP); always has ID 0
	// When the attack is executed, the mainEMP is spawned instantly. Its spawn type's time is ignored.
	std::shared_ptr<EditorMovablePoint> mainEMP;
	// Whether or not to play the enemy's attack animation with this attack
	bool playAttackAnimation;
};