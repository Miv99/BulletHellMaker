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
	EditorAttack(std::shared_ptr<EditorAttack> copy);

	std::string format() override;
	void load(std::string formattedString) override;

	bool legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::string& message);

	void loadEMPBulletModels(const LevelPack& levelPack);

	/*
	Executes the attack as an enemy.

	entity - the enemy that is executing this attack
	timeLag - the time elapsed since this attack was supposed to execute
	attackPatternID - the ID of the enemy's current attack pattern
	enemyID - the ID of the enemy executing this attack
	enemyPhaseID - the ID of the enemy's current phase 
	*/
	void EditorAttack::executeAsEnemy(EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry& registry, uint32_t entity, float timeLag, int attackPatternID, int enemyID, int enemyPhaseID);
	/*
	Executes the attack as a player.

	entity - the enemy that is executing this attack
	timeLag - the time elapsed since this attack was supposed to execute
	attackPatternID - the ID of the enemy's current attack pattern
	*/
	void EditorAttack::executeAsPlayer(EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry& registry, uint32_t entity, float timeLag, int attackPatternID);

	inline int getID() { return id; }
	inline std::string getName() { return name; }
	inline bool getPlayAttackAnimation() { return playAttackAnimation; }
	inline std::shared_ptr<EditorMovablePoint> getMainEMP() { return mainEMP; }
	inline void setName(std::string name) { this->name = name; }
	inline void setPlayAttackAnimation(bool playAttackAnimation) { this->playAttackAnimation = playAttackAnimation; }
	
	float searchLargestHitbox() const;
	// Search for the EMP with the ID
	std::shared_ptr<EditorMovablePoint> searchEMP(int id);

	/*
	Generates a list of string vectors such that, when each all the string vectors are added to a tgui::TreeView,
	the tree hierarchy of the EMPs of this attack is created. Each entry is an EMP's ID.

	nodeText - a function that takes an EMP and returns a string -- the text in the tgui::TreeView for the node for that EMP
	*/
	std::vector<std::vector<sf::String>> generateTreeViewEmpHierarchy(std::function<sf::String(const EditorMovablePoint&)> nodeText);

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