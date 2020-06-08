#pragma once
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <tuple>
#include "TextMarshallable.h"
#include "EnemyPhase.h"
#include "EnemyPhaseStartCondition.h"
#include "EntityAnimatableSet.h"
#include "Animatable.h"
#include "DeathAction.h"
#include "Components.h"

/*
An enemy in the editor.
Spawned by EnemySpawn.

The first PhaseStartCondition must be set such that the enemy is always in a phase.
*/
class EditorEnemy : public TextMarshallable {
public:
	inline EditorEnemy() {}
	inline EditorEnemy(int id) : id(id) {}

	std::string format() const override;
	void load(std::string formattedString) override;

	bool legal(std::string& message) const;

	inline int getID() const { return id; }
	inline std::tuple<std::shared_ptr<EnemyPhaseStartCondition>, int, EntityAnimatableSet> getPhaseData(int index) const { return phaseIDs[index]; }
	inline int getPhasesCount() const { return phaseIDs.size(); }
	inline std::string getName() const { return name; }
	inline float getHitboxRadius() const { return hitboxRadius; }
	inline int getHealth() const { return health; }
	inline float getDespawnTime() const { return despawnTime; }
	inline const std::vector<std::shared_ptr<DeathAction>> getDeathActions() const { return deathActions; }
	inline bool getIsBoss() const { return isBoss; }
	// Returns a reference
	inline SoundSettings& getHurtSound() { return hurtSound; }
	// Returns a reference
	inline SoundSettings& getDeathSound() { return deathSound; }
	inline bool usesEnemyPhase(int enemyPhaseID) const { return enemyPhaseCount.count(enemyPhaseID) > 0 && enemyPhaseCount.at(enemyPhaseID) > 0; }

	inline void addDeathAction(std::shared_ptr<DeathAction> action) { deathActions.push_back(action); }
	inline void removeDeathAction(int index) { deathActions.erase(deathActions.begin() + index); }

	inline void setName(std::string name) { this->name = name; }
	inline void setHitboxRadius(float hitboxRadius) { this->hitboxRadius = hitboxRadius; }
	inline void setHealth(int health) { this->health = health; }
	inline void setDespawnTime(float despawnTime) { this->despawnTime = despawnTime; }
	inline void setIsBoss(bool isBoss) { this->isBoss = isBoss; }

	inline void addPhaseID(int index, std::shared_ptr<EnemyPhaseStartCondition> startCondition, int phaseID, EntityAnimatableSet animatableSet) {
		phaseIDs.insert(phaseIDs.begin() + index, std::make_tuple(startCondition, phaseID, animatableSet));

		if (enemyPhaseCount.count(phaseID) == 0) {
			enemyPhaseCount[phaseID] = 1;
		} else {
			enemyPhaseCount[phaseID]++;
		}
	}
	inline void removePhaseID(int index) {
		int phaseID = std::get<1>(phaseIDs[index]);
		phaseIDs.erase(phaseIDs.begin() + index);

		if (enemyPhaseCount.count(phaseID) == 0) {
			enemyPhaseCount[phaseID] = 1;
		} else {
			enemyPhaseCount[phaseID]++;
		}
	}

private:
	// ID of the enemy
	int id;
	// User-defined name of the enemy
	std::string name;
	// Radius of the hitbox associated with this enemy
	float hitboxRadius;
	// Health and maximum health of this enemy
	int health;
	// Time it takes for this enemy to despawn. Set < 0 if it should not despawn
	float despawnTime = -1;
	// Tuple of: the condition to start the phase, the phase ID, and the animatable set used by the enenemy while in that phase
	// The first phase must have a TimeBasedEnemyPhaseStartCondition with t=0 to ensure that the phase can start as soon as the enemy spawns
	std::vector<std::tuple<std::shared_ptr<EnemyPhaseStartCondition>, int, EntityAnimatableSet>> phaseIDs;
	// Death actions. This should not include the death animation because the death animation is dependent on the current phase.
	std::vector<std::shared_ptr<DeathAction>> deathActions;

	bool isBoss = false;

	SoundSettings hurtSound;
	SoundSettings deathSound;

	// Maps EditorEnemyPhaseID to the number of times it appears in phaseIDs.
	// This map is not saved in format() but is reconstructed in load().
	std::map<int, int> enemyPhaseCount;
};