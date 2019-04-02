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

	std::string format() override;
	void load(std::string formattedString) override;

	bool legal(SpriteLoader& spriteLoader, std::string& message);

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

	inline void addSoundEffect(float time, std::string fileName) {
		auto item = std::make_pair(time, fileName);
		auto it = upper_bound(soundEffectNames.begin(), soundEffectNames.end(), item, [](auto& lhs, auto& rhs)->bool { return lhs.first < rhs.first; });
		soundEffectNames.insert(it, item);
	}

	inline int getID() { return id; }
	inline std::string getName() { return name; }
	inline bool getPlayAttackAnimation() { return playAttackAnimation; }
	inline void setName(std::string name) { this->name = name; }
	inline void setPlayAttackAnimation(bool playAttackAnimation) { this->playAttackAnimation = playAttackAnimation; }
	
	float searchLargestHitbox();
	// Search for the EMP with the ID
	std::shared_ptr<EditorMovablePoint> searchEMP(int id);

private:
	// The ID of the next EMP created in this attack. Not saved.
	int nextEMPID = 0;

	// ID of the attack
	int id;
	// User-defined name of the attack
	std::string name;
	// Root of the EMP tree (the main EMP); always has ID 0
	// When the attack is executed, the mainEMP is spawned instantly. Its spawn type's time is ignored.
	std::shared_ptr<EditorMovablePoint> mainEMP;
	// List of file names of sound effects to be played and when (t=0 is start of the attack)
	std::vector<std::pair<float, std::string>> soundEffectNames;
	// Whether or not to play the enemy's attack animation with this attack
	bool playAttackAnimation;
};