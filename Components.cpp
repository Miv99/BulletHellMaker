#include "Components.h"
#include "MovablePoint.h"
#include "LevelPack.h"
#include "EditorMovablePointAction.h"
#include "AttackPattern.h"
#include "Enemy.h"
#include "EnemyPhase.h"
#include "Attack.h"
#include "EditorMovablePointSpawnType.h"
#include "EnemySpawn.h"
#include "EntityCreationQueue.h"
#include "EditorMovablePoint.h"
#include <math.h>

sf::Vector2f MovementPathComponent::update(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float deltaTime) {
	time += deltaTime;
	// While loop for actions with lifespan of 0 like DetachFromParent 
	while (currentActionsIndex < actions.size() && time >= path->getLifespan()) {
		time -= path->getLifespan();
		path = actions[currentActionsIndex]->execute(queue, registry, entity, time);
		currentActionsIndex++;
	}
	if (useReferenceEntity) {
		auto& pos = registry.get<PositionComponent>(referenceEntity);
		return path->compute(sf::Vector2f(pos.getX(), pos.getY()), time);
	} else {
		return path->compute(sf::Vector2f(0, 0), time);
	}
}

void MovementPathComponent::setActions(std::vector<std::shared_ptr<EMPAction>> actions) {
	this->actions = actions;

	currentActionsIndex = 0;
	time = 0;
}

void MovementPathComponent::initialSpawn(const entt::DefaultRegistry& registry, uint32_t entity, std::shared_ptr<EMPSpawnType> spawnType, std::vector<std::shared_ptr<EMPAction>>& actions) {
	auto spawnInfo = spawnType->getSpawnInfo(registry, entity);
	useReferenceEntity = spawnInfo.useReferenceEntity;
	referenceEntity = spawnInfo.referenceEntity;

	path = std::make_shared<StationaryMP>(spawnInfo.position, 0);
}

EnemyComponent::EnemyComponent(std::shared_ptr<EditorEnemy> enemyData, int enemyID) : enemyData(enemyData), enemyID(enemyID) {}

void EnemyComponent::update(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, uint32_t entity, float deltaTime) {
	timeSinceSpawned += deltaTime;
	timeSincePhase += deltaTime;
	timeSinceAttackPattern += deltaTime;

	checkAttacks(queue, spriteLoader, levelPack, registry, entity);
	checkAttackPatterns(queue, spriteLoader, levelPack, registry, entity);
	checkPhases(queue, spriteLoader, levelPack, registry, entity);
}

void EnemyComponent::checkPhases(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, uint32_t entity) {
	// Check if entity can continue to next phase
	// While loop so that enemy can skip phases
	while (currentPhaseIndex + 1 < enemyData->getPhasesCount()) {
		auto nextPhaseData = enemyData->getPhaseData(currentPhaseIndex + 1);
		// Check if condition for next phase is satisfied
		if (nextPhaseData.first->satisfied(registry, entity)) {
			// Current phase ends, so call its ending EnemyPhaseAction
			if (currentPhase && currentPhase->getPhaseEndAction()) {
				currentPhase->getPhaseEndAction()->execute(registry, entity);
			}

			timeSincePhase = 0;
			timeSinceAttackPattern = 0;

			currentPhaseIndex++;
			currentAttackPatternIndex = -1;
			currentAttackIndex = -1;

			currentPhase = levelPack.getEnemyPhase(nextPhaseData.second);
			currentAttackPattern = nullptr;

			// Next phase begins, so call its beginning EnemyPhaseAction
			if (currentPhase->getPhaseBeginAction()) {
				currentPhase->getPhaseBeginAction()->execute(registry, entity);
			}

			checkAttackPatterns(queue, spriteLoader, levelPack, registry, entity);
		} else {
			break;
		}
	}
}

void EnemyComponent::checkAttackPatterns(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, uint32_t entity) {
	// Attack patterns loop, so entity can always continue to the next attack pattern
	while (currentPhase) {
		auto nextAttackPattern = currentPhase->getAttackPatternData(currentAttackPatternIndex + 1);
		// Check if condition for next attack pattern is satisfied
		if (timeSincePhase >= nextAttackPattern.first) {
			timeSinceAttackPattern = timeSincePhase - nextAttackPattern.first;

			currentAttackPatternIndex++;
			currentAttackIndex = -1;

			currentAttackPattern = levelPack.getAttackPattern(nextAttackPattern.second);

			currentAttackPattern->changeEntityPathToAttackPatternActions(registry, entity, timeSinceAttackPattern);
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
		if (timeSinceAttackPattern >= nextAttack.first) {
			currentAttackIndex++;
			levelPack.getAttack(nextAttack.second)->executeAsEnemy(queue, spriteLoader, registry, entity, timeSinceAttackPattern - nextAttack.first, currentAttackPattern->getID(), enemyID, currentPhase->getID());
		} else {
			break;
		}
	}
}

void LevelManagerTag::update(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, float deltaTime) {
	timeSinceStartOfLevel += deltaTime;
	timeSinceLastEnemySpawn += deltaTime;

	while (currentEnemyGroupSpawnIndex + 1 < level->getEnemyGroupsCount()) {
		if (level->conditionSatisfied(currentEnemyGroupSpawnIndex + 1, registry)) {
			for (EnemySpawnInfo info : level->getEnemyGroupSpawnInfo(currentEnemyGroupSpawnIndex + 1)) {
				info.spawnEnemy(spriteLoader, levelPack, registry, queue);
			}

			currentEnemyGroupSpawnIndex++;
		} else {
			break;
		}
	}
}

void SpriteComponent::update(float deltaTime) {
	if (animation != nullptr) {
		animation->update(deltaTime);
	}
}

EMPSpawnerComponent::EMPSpawnerComponent(std::vector<std::shared_ptr<EditorMovablePoint>> emps, uint32_t parent, int attackID, int attackPatternID, int enemyID, int enemyPhaseID) : parent(parent), attackID(attackID), attackPatternID(attackPatternID), enemyID(enemyID), enemyPhaseID(enemyPhaseID) {
	for (auto emp : emps) {
		this->emps.push(emp);
	}
}

EMPSpawnerComponent::EMPSpawnerComponent(std::vector<std::shared_ptr<EditorMovablePoint>> emps, uint32_t parent, int attackID, int attackPatternID) : parent(parent), attackID(attackID), attackPatternID(attackPatternID) {
	for (auto emp : emps) {
		this->emps.push(emp);
	}
}

void EMPSpawnerComponent::update(entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, EntityCreationQueue& queue, float deltaTime) {
	time += deltaTime;

	if (isEnemyBulletSpawner) {
		while (!emps.empty()) {
			float t = emps.front()->getSpawnType()->getTime();
			if (time >= t) {
				queue.addCommand(std::make_unique<EMPSpawnFromEnemyCommand>(registry, spriteLoader, emps.front(), parent, time - t, attackID, attackPatternID, enemyID, enemyPhaseID));
				emps.pop();
			} else {
				break;
			}
		}
	} else {
		while (!emps.empty()) {
			float t = emps.front()->getSpawnType()->getTime();
			if (time >= t) {
				queue.addCommand(std::make_unique<EMPSpawnFromPlayerCommand>(registry, spriteLoader, emps.front(), parent, time - t, attackID, attackPatternID));
				emps.pop();
			} else {
				break;
			}
		}
	}
}
