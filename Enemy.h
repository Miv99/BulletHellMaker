#pragma once
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include "TextMarshallable.h"
#include "EnemyPhase.h"
#include "EnemyPhaseStartCondition.h"

/*
An enemy in the editor.
Spawned by EnemySpawn.

The first PhaseStartCondition must be set such that the enemy is always in a phase.
*/
class EditorEnemy : public TextMarshallable {
public:
	inline EditorEnemy() {}
	inline EditorEnemy(int id) : id(id) {}

	std::string format() override;
	void load(std::string formattedString) override;

	inline int getID() { return id; }
	inline std::pair<std::shared_ptr<EnemyPhaseStartCondition>, int> getPhaseData(int index) { return phaseIDs[index]; }
	inline int getPhasesCount() { return phaseIDs.size(); }
	inline std::string getName() { return name; }
	inline std::string getSpriteName() { return spriteName; }
	inline std::string getSpriteSheetName() { return spriteSheetName; }
	inline float getHitboxRadius() { return hitboxRadius; }
	inline float getHealth() { return health; }

	inline void setName(std::string name) { this->name = name; }
	inline void setSpriteName(std::string spriteName, std::string spriteSheetName) { this->spriteName = spriteName; this->spriteSheetName = spriteSheetName; }
	inline void setHitboxRadius(float hitboxRadius) { this->hitboxRadius = hitboxRadius; }
	inline void setHealth(float health) { this->health = health; }
	inline void addPhaseID(int index, std::shared_ptr<EnemyPhaseStartCondition> startCondition, int phaseID) {
		phaseIDs.insert(phaseIDs.begin() + index, std::make_pair(startCondition, phaseID));
	}
	inline void removePhaseID(int index) {
		phaseIDs.erase(phaseIDs.begin() + index);
	}

private:
	// ID of the enemy
	int id;
	// User-defined name of the enemy
	std::string name;
	// Name of the sprite associated with this enemy
	std::string spriteName;
	// Name of the sprite sheet the sprite is in
	std::string spriteSheetName;
	// Radius of the hitbox associated with this enemy
	float hitboxRadius;
	// Health and maximum health of this enemy
	float health;
	// Phase ids and the condition for those phases to activate
	std::vector<std::pair<std::shared_ptr<EnemyPhaseStartCondition>, int>> phaseIDs;
};