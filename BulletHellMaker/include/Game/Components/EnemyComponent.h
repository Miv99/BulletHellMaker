#pragma once
#include <entt/entt.hpp>

class EditorEnemy;
class EditorEnemyPhase;
class EnemyPhaseStartCondition;
class EditorAttackPattern;
class EnemySpawnInfo;
class LevelPack;
class EntityCreationQueue;
class SpriteLoader;
class DeathAction;

/*
Component for an enemy.
*/
class EnemyComponent {
public:
	EnemyComponent(std::shared_ptr<EditorEnemy> enemyData, std::shared_ptr<EnemySpawnInfo> spawnInfo, int enemyID);
	void update(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, uint32_t entity, float deltaTime);

	inline float getTimeSinceSpawned() const { return timeSinceSpawned; }
	inline float getTimeSinceLastPhase() const { return timeSincePhase; }
	inline std::shared_ptr<EditorEnemy> getEnemyData() const { return enemyData; }
	inline std::shared_ptr<EnemySpawnInfo> getEnemySpawnInfo() const { return spawnInfo; }
	/*
	Returns the PlayAnimatableDeathAction associated with the current phase.
	*/
	std::shared_ptr<DeathAction> getCurrentDeathAnimationAction();
	/*
	Returns the signal that is emitted whenever phase changes.
	Parameters: this component's entity, the new phase, the start condition of the old 
	phase (nullptr if the new phase is the first phase), and the next phase's start condition (nullptr if next phase is the last phase)
	*/
	std::shared_ptr<entt::SigH<void(uint32_t, std::shared_ptr<EditorEnemyPhase>, std::shared_ptr<EnemyPhaseStartCondition>, std::shared_ptr<EnemyPhaseStartCondition>)>> getEnemyPhaseChangeSignal();

private:
	// Time since being spawned
	float timeSinceSpawned = 0;
	// Time since start of the current phase
	float timeSincePhase = 0;
	// Time since start of the current attack pattern
	float timeSinceAttackPattern = 0;

	std::shared_ptr<EnemySpawnInfo> spawnInfo;

	int enemyID;
	std::shared_ptr<EditorEnemy> enemyData;
	std::shared_ptr<EditorEnemyPhase> currentPhase = nullptr;
	std::shared_ptr<EditorAttackPattern> currentAttackPattern = nullptr;

	// Whether to keep checking for if the entity can continue to the next attack pattern
	bool checkNextAttackPattern = true;

	// Current phase index in list of phases in EditorEnemy
	int currentPhaseIndex = -1;
	// Current attack pattern index in list of attack patterns in current EditorEnemyPhase
	int currentAttackPatternIndex = -1;
	// Current attack index in list of attacks in current EditorAttackPattern
	int currentAttackIndex = -1;

	// Emitted whenever phase changes.
	// Parameters: this component's entity, the new phase, the start condition of the old 
	// phase (nullptr if the new phase is the first phase), and the next phase's start condition (nullptr if next phase is the last phase)
	std::shared_ptr<entt::SigH<void(uint32_t, std::shared_ptr<EditorEnemyPhase>, std::shared_ptr<EnemyPhaseStartCondition>, std::shared_ptr<EnemyPhaseStartCondition>)>> enemyPhaseChangeSignal;

	// Check for any missed phases/attack patterns/attacks since the last update and then executes their relevant actions
	void checkPhases(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, uint32_t entity);
	void checkAttackPatterns(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, uint32_t entity);
	void checkAttacks(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, uint32_t entity);
};