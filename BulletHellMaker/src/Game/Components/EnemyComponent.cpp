#include <Game/Components/EnemyComponent.h>

#include <DataStructs/SpriteLoader.h>
#include <LevelPack/LevelPack.h>
#include <LevelPack/Enemy.h>
#include <LevelPack/EnemyPhase.h>
#include <LevelPack/EnemyPhaseStartCondition.h>
#include <LevelPack/AttackPattern.h>
#include <LevelPack/Attack.h>
#include <LevelPack/EnemySpawn.h>
#include <LevelPack/DeathAction.h>
#include <Game/EntityCreationQueue.h>

EnemyComponent::EnemyComponent(std::shared_ptr<EditorEnemy> enemyData, std::shared_ptr<EnemySpawnInfo> spawnInfo, int enemyID) 
	: enemyData(enemyData), spawnInfo(spawnInfo), enemyID(enemyID) {
}

void EnemyComponent::update(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, uint32_t entity, float deltaTime) {
	timeSinceSpawned += deltaTime;
	timeSincePhase += deltaTime;
	timeSinceAttackPattern += deltaTime;

	checkAttacks(queue, spriteLoader, levelPack, registry, entity);
	checkAttackPatterns(queue, spriteLoader, levelPack, registry, entity);
	checkPhases(queue, spriteLoader, levelPack, registry, entity);
}

std::shared_ptr<DeathAction> EnemyComponent::getCurrentDeathAnimationAction() {
	assert(currentPhaseIndex != -1);
	return std::get<2>(enemyData->getPhaseData(currentPhaseIndex)).getDeathAction();
}

std::shared_ptr<entt::SigH<void(uint32_t, std::shared_ptr<EditorEnemyPhase>, std::shared_ptr<EnemyPhaseStartCondition>, std::shared_ptr<EnemyPhaseStartCondition>)>> EnemyComponent::getEnemyPhaseChangeSignal() {
	if (enemyPhaseChangeSignal) {
		return enemyPhaseChangeSignal;
	}
	enemyPhaseChangeSignal = std::make_shared<entt::SigH<void(uint32_t, std::shared_ptr<EditorEnemyPhase>, std::shared_ptr<EnemyPhaseStartCondition>, std::shared_ptr<EnemyPhaseStartCondition>)>>();
	return enemyPhaseChangeSignal;
}

void EnemyComponent::checkPhases(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, uint32_t entity) {
	// Check if entity can continue to next phase
	// While loop so that enemy can skip phases
	while (currentPhaseIndex + 1 < enemyData->getPhasesCount()) {
		auto nextPhaseData = enemyData->getPhaseData(currentPhaseIndex + 1);
		// Check if condition for next phase is satisfied
		if (std::get<0>(nextPhaseData)->satisfied(registry, entity)) {
			// Current phase ends, so call its ending EnemyPhaseAction
			if (currentPhase && currentPhase->getPhaseEndAction()) {
				currentPhase->getPhaseEndAction()->execute(registry, entity);
			}

			timeSincePhase = 0;
			timeSinceAttackPattern = 0;

			currentPhaseIndex++;
			currentAttackPatternIndex = -1;
			currentAttackIndex = -1;

			currentPhase = levelPack.getGameplayEnemyPhase(std::get<1>(nextPhaseData), std::get<4>(nextPhaseData));
			currentAttackPattern = nullptr;

			// Do not loop through attack patterns while in this current phase if no attack pattern in this phase takes longer than 0 secoonds
			// to execute, to prevent an infinite loop in checkAttackPatterns()
			// Loop to (currentPhase->getAttackPatternsCount() * 2) to check for any loop delays as well
			checkNextAttackPattern = false;
			for (int i = 0; i < currentPhase->getAttackPatternsCount() * 2; i++) {
				if (std::get<0>(currentPhase->getAttackPatternData(levelPack, i)) > 0) {
					checkNextAttackPattern = true;
					break;
				}
			}

			// Send phase change signal
			if (enemyPhaseChangeSignal) {
				if (currentPhaseIndex == 0) {
					if (currentPhaseIndex + 1 == enemyData->getPhasesCount()) {
						enemyPhaseChangeSignal->publish(entity, currentPhase, nullptr, nullptr);
					} else {
						enemyPhaseChangeSignal->publish(entity, currentPhase, nullptr, std::get<0>(enemyData->getPhaseData(currentPhaseIndex + 1)));
					}
				} else {
					if (currentPhaseIndex + 1 == enemyData->getPhasesCount()) {
						enemyPhaseChangeSignal->publish(entity, currentPhase, std::get<0>(enemyData->getPhaseData(currentPhaseIndex - 1)), nullptr);
					} else {
						enemyPhaseChangeSignal->publish(entity, currentPhase, std::get<0>(enemyData->getPhaseData(currentPhaseIndex - 1)), std::get<0>(enemyData->getPhaseData(currentPhaseIndex + 1)));
					}
				}
			}

			// Next phase begins, so call its beginning EnemyPhaseAction
			if (currentPhase->getPhaseBeginAction()) {
				currentPhase->getPhaseBeginAction()->execute(registry, entity);
			}

			// Set entity's animatable set
			registry.get<AnimatableSetComponent>(entity).setAnimatableSet(std::get<2>(nextPhaseData));

			// Play the new music, if any
			if (currentPhase->getPlayMusic()) {
				levelPack.playMusic(currentPhase->getMusicSettings());
			}

			checkAttackPatterns(queue, spriteLoader, levelPack, registry, entity);
		} else {
			break;
		}
	}
}

void EnemyComponent::checkAttackPatterns(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, uint32_t entity) {
	// Attack patterns loop, so entity can always continue to the next attack pattern
	while (currentPhase && checkNextAttackPattern) {
		auto nextAttackPattern = currentPhase->getAttackPatternData(levelPack, currentAttackPatternIndex + 1);
		// Check if condition for next attack pattern is satisfied
		if (timeSincePhase >= std::get<0>(nextAttackPattern)) {
			timeSinceAttackPattern = timeSincePhase - std::get<0>(nextAttackPattern);

			currentAttackPatternIndex++;
			currentAttackIndex = -1;

			currentAttackPattern = levelPack.getGameplayAttackPattern(std::get<1>(nextAttackPattern), std::get<2>(nextAttackPattern));

			currentAttackPattern->changeEntityPathToAttackPatternActions(queue, registry, entity, timeSinceAttackPattern);

			// Update shadow trail properties
			auto& shadowTrail = registry.get<ShadowTrailComponent>(entity);
			shadowTrail.setInterval(currentAttackPattern->getShadowTrailInterval());
			shadowTrail.setLifespan(currentAttackPattern->getShadowTrailLifespan());

			checkAttacks(queue, spriteLoader, levelPack, registry, entity);
		} else {
			break;
		}
	}
}

void EnemyComponent::checkAttacks(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, uint32_t entity) {
	// Check if entity can continue to next attack
	while (currentAttackPattern && currentAttackIndex + 1 < currentAttackPattern->getAttacksCount()) {
		auto nextAttack = currentAttackPattern->getAttackData(currentAttackIndex + 1);
		if (timeSinceAttackPattern >= std::get<0>(nextAttack)) {
			currentAttackIndex++;
			levelPack.getGameplayAttack(std::get<1>(nextAttack), std::get<2>(nextAttack))->executeAsEnemy(queue, spriteLoader, registry, entity, timeSinceAttackPattern - std::get<0>(nextAttack), currentAttackPattern->getID(), enemyID, currentPhase->getID());
		} else {
			break;
		}
	}
}