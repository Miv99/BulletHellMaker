#include "EntityCreationQueue.h"
#include "EditorMovablePoint.h"
#include "EditorMovablePointAction.h"
#include "EditorMovablePointSpawnType.h"
#include "SpriteLoader.h"
#include "Enemy.h"
#include "EntityAnimatableSet.h"
#include <algorithm>
#include <iostream>

EMPSpawnFromEnemyCommand::EMPSpawnFromEnemyCommand(entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, std::shared_ptr<EditorMovablePoint> emp, uint32_t entity, float timeLag, int attackID, int attackPatternID, int enemyID, int enemyPhaseID, bool playAttackAnimation) :
	EntityCreationCommand(registry), spriteLoader(spriteLoader), emp(emp), playAttackAnimation(playAttackAnimation),
	entity(entity), timeLag(timeLag), attackID(attackID), attackPatternID(attackPatternID), enemyID(enemyID), enemyPhaseID(enemyPhaseID) {
}

void EMPSpawnFromEnemyCommand::execute(EntityCreationQueue& queue) {
	// Change AnimatableSetComponent state to attack state
	if (playAttackAnimation) {
		registry.get<AnimatableSetComponent>(entity).changeState(AnimatableSetComponent::ATTACK, spriteLoader, registry.get<SpriteComponent>(entity));
	}

	// Create the entity
	auto bullet = registry.create();
	MPSpawnInformation spawnInfo = emp->getSpawnType()->getSpawnInfo(registry, entity);

	if (spawnInfo.useReferenceEntity) {
		// If the bullet's MP reference is the entity executing the attack, make sure the bullet despawns along with the entity
		if (spawnInfo.referenceEntity == entity) {
			registry.assign<DespawnComponent>(bullet, entity, false);
		}

		// Spawn at correct position
		auto referencePos = registry.get<PositionComponent>(spawnInfo.referenceEntity);
		registry.assign<PositionComponent>(bullet, referencePos.getX() + spawnInfo.position.x, referencePos.getY() + spawnInfo.position.y);
	} else {
		registry.assign<PositionComponent>(bullet, spawnInfo.position.x, spawnInfo.position.y);
	}

	if (registry.has<DespawnComponent>(bullet)) {
		if (emp->getDespawnTime() > 0) {
			registry.get<DespawnComponent>(bullet).setMaxTime(std::min(emp->getTotalPathTime(), emp->getDespawnTime()));
		} else {
			registry.get<DespawnComponent>(bullet).setMaxTime(emp->getTotalPathTime());
		}
	} else {
		if (emp->getDespawnTime() > 0) {
			registry.assign<DespawnComponent>(bullet, std::min(emp->getTotalPathTime(), emp->getDespawnTime()));
		} else {
			registry.assign<DespawnComponent>(bullet, emp->getTotalPathTime());
		}
	}

	registry.assign<MovementPathComponent>(bullet, queue, bullet, registry, entity, emp->getSpawnType(), emp->getActions(), timeLag);
	// If animatable name is not empty, this EMP is a bullet
	Animatable animatable = emp->getAnimatable();
	if (animatable.getAnimatableName() != "") {
		registry.assign<HitboxComponent>(bullet, emp->getHitboxRadius(), emp->getHitboxPosX(), emp->getHitboxPosY());
		registry.assign<SpriteComponent>(bullet, spriteLoader.getSprite(animatable.getAnimatableName(), animatable.getSpriteSheetName()));

		registry.assign<EnemyBulletComponent>(bullet, attackID, attackPatternID, enemyID, enemyPhaseID);
	}

	if (emp->getShadowTrailLifespan() > 0) {
		registry.assign<ShadowTrailComponent>(bullet, emp->getShadowTrailInterval(), emp->getShadowTrailLifespan());
	}

	if (emp->getChildren().size() > 0) {
		EMPSpawnerComponent& empSpawnerComponent = registry.assign<EMPSpawnerComponent>(bullet, emp->getChildren(), bullet, attackID, attackPatternID, enemyID, enemyPhaseID, playAttackAnimation);
		// Update in case there are any children that should be spawned instantly
		empSpawnerComponent.update(registry, spriteLoader, queue, 0);
	}
}

int EMPSpawnFromEnemyCommand::getEntitiesQueuedCount() {
	return emp->getTreeSize();
}

EMPSpawnFromPlayerCommand::EMPSpawnFromPlayerCommand(entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, std::shared_ptr<EditorMovablePoint> emp, uint32_t entity, float timeLag, int attackID, int attackPatternID, bool playAttackAnimation) :
	EntityCreationCommand(registry), spriteLoader(spriteLoader), emp(emp), playAttackAnimation(playAttackAnimation),
	entity(entity), timeLag(timeLag), attackID(attackID), attackPatternID(attackPatternID) {
}

void EMPSpawnFromPlayerCommand::execute(EntityCreationQueue& queue) {
	// Change AnimatableSetComponent state to attack state
	if (playAttackAnimation) {
		registry.get<AnimatableSetComponent>(entity).changeState(AnimatableSetComponent::ATTACK, spriteLoader, registry.get<SpriteComponent>(entity));
	}
	
	// Create the entity
	auto bullet = registry.create();
	MPSpawnInformation spawnInfo = emp->getSpawnType()->getSpawnInfo(registry, entity);

	if (spawnInfo.useReferenceEntity) {
		// If the bullet's MP reference is the entity executing the attack, make sure the bullet despawns along with the entity
		if (spawnInfo.referenceEntity == entity) {
			registry.assign<DespawnComponent>(bullet, entity, false);
		}

		// Spawn at correct position
		auto referencePos = registry.get<PositionComponent>(spawnInfo.referenceEntity);
		registry.assign<PositionComponent>(bullet, referencePos.getX() + spawnInfo.position.x, referencePos.getY() + spawnInfo.position.y);
	} else {
		registry.assign<PositionComponent>(bullet, spawnInfo.position.x, spawnInfo.position.y);
	}

	registry.assign<MovementPathComponent>(bullet, queue, bullet, registry, entity, emp->getSpawnType(), emp->getActions(), timeLag);
	// If animatable name is not empty, this EMP is a bullet
	Animatable animatable = emp->getAnimatable();
	if (animatable.getAnimatableName() != "") {
		registry.assign<HitboxComponent>(bullet, emp->getHitboxRadius(), emp->getHitboxPosX(), emp->getHitboxPosY());
		registry.assign<SpriteComponent>(bullet, spriteLoader.getSprite(animatable.getAnimatableName(), animatable.getSpriteSheetName()));

		registry.assign<PlayerBulletComponent>(bullet, attackID, attackPatternID);
	}

	if (emp->getShadowTrailLifespan() > 0) {
		registry.assign<ShadowTrailComponent>(bullet, emp->getShadowTrailInterval(), emp->getShadowTrailLifespan());
	}

	if (emp->getChildren().size() > 0) {
		EMPSpawnerComponent& empSpawnerComponent = registry.assign<EMPSpawnerComponent>(bullet, emp->getChildren(), bullet, attackID, attackPatternID, playAttackAnimation);
		// Update in case there are any children that should be spawned instantly
		empSpawnerComponent.update(registry, spriteLoader, queue, 0);
	}
}

int EMPSpawnFromPlayerCommand::getEntitiesQueuedCount() {
	return emp->getTreeSize();
}

void EMPADetachFromParentCommand::execute(EntityCreationQueue& queue) {
	auto& mpc = registry.get<MovementPathComponent>(entity);

	// Make new reference entity
	auto reference = registry.create();
	registry.assign<PositionComponent>(reference, lastPosX, lastPosY);
	// Reference despawns when the entity executing this action despawns
	registry.assign<DespawnComponent>(reference, entity, false);
	mpc.setReferenceEntity(reference);
	// Update position
	registry.get<PositionComponent>(entity).setPosition(mpc.getPath()->compute(sf::Vector2f(lastPosX, lastPosY), mpc.getTime()));
}

int EMPADetachFromParentCommand::getEntitiesQueuedCount() {
	return 1;
}

void CreateMovementRefereceEntityCommand::execute(EntityCreationQueue& queue) {
	auto& mpc = registry.get<MovementPathComponent>(entity);

	// Make new reference entity
	// The new reference entity is attached to [entity executing this action]'s reference
	// If no reference, it is attached to nothing
	auto reference = registry.create();
	registry.assign<SimpleEMPReferenceComponent>(reference);
	registry.assign<PositionComponent>(reference, lastPosX, lastPosY);
	if (mpc.usesReferenceEntity()) {
		// Find base reference (the first entity in the reference chain that does not have a SimpleEMPReferenceComponent or has no reference)
		uint32_t baseReference = mpc.getReferenceEntity();
		while (registry.has<SimpleEMPReferenceComponent>(baseReference)) {
			auto& brMPC = registry.get<MovementPathComponent>(baseReference);
			if (brMPC.usesReferenceEntity()) {
				auto temp = brMPC.getReferenceEntity();

				// Delete current base reference
				// Each SimpleEMPReference can be a reference for only one entity, so this deletion is safe
				registry.destroy(baseReference);

				baseReference = temp;
			} else {
				break;
			}
		}
		// Calculate position of new reference
		auto& brLastPos = registry.get<PositionComponent>(baseReference);

		registry.assign<MovementPathComponent>(reference, queue, reference, registry, baseReference, std::make_shared<EnemyAttachedEMPSpawn>(0, lastPosX - brLastPos.getX(), lastPosY - brLastPos.getY()), std::vector<std::shared_ptr<EMPAction>>(), timeLag);
	} else {
		registry.assign<MovementPathComponent>(reference, queue, reference, registry, reference, std::make_shared<SpecificGlobalEMPSpawn>(0, lastPosX, lastPosY), std::vector<std::shared_ptr<EMPAction>>(), timeLag);
	}

	// Reference despawns when the entity executing this action despawns
	registry.assign<DespawnComponent>(reference, entity, false);

	mpc.setReferenceEntity(reference);
	// Update position
	mpc.update(queue, registry, entity, registry.get<PositionComponent>(entity), 0);
}

int CreateMovementRefereceEntityCommand::getEntitiesQueuedCount() {
	return 1;
}

SpawnEnemyCommand::SpawnEnemyCommand(entt::DefaultRegistry & registry, SpriteLoader & spriteLoader, std::shared_ptr<EditorEnemy> enemyInfo, float x, float y) : 
	EntityCreationCommand(registry), spriteLoader(spriteLoader), enemyInfo(enemyInfo), x(x), y(y) {
}

void SpawnEnemyCommand::execute(EntityCreationQueue & queue) {
	auto enemy = registry.create();
	registry.assign<PositionComponent>(enemy, x, y);
	// EMPActions vector empty because it will be populated in EnemySystem when the enemy begins a phase
	registry.assign<MovementPathComponent>(enemy, queue, enemy, registry, enemy, std::make_shared<SpecificGlobalEMPSpawn>(0, x, y), std::vector<std::shared_ptr<EMPAction>>(), 0);
	registry.assign<HealthComponent>(enemy, enemyInfo->getHealth(), enemyInfo->getHealth());
	if (enemyInfo->getDespawnTime() > 0) {
		registry.assign<DespawnComponent>(enemy, enemyInfo->getDespawnTime());
	}
	registry.assign<HitboxComponent>(enemy, enemyInfo->getHitboxRadius(), enemyInfo->getHitboxPosX(), enemyInfo->getHitboxPosY());
	registry.assign<SpriteComponent>(enemy);
	registry.assign<EnemyComponent>(enemy, enemyInfo, enemyInfo->getID());
	registry.assign<AnimatableSetComponent>(enemy);

	// Reset level manager's timeSinceLastEnemySpawn
	registry.get<LevelManagerTag>().setTimeSinceLastEnemySpawn(0);
}

int SpawnEnemyCommand::getEntitiesQueuedCount() {
	return 1;
}

void SpawnShadowTrailCommand::execute(EntityCreationQueue & queue) {
	auto shadow = registry.create();
	registry.assign<PositionComponent>(shadow, x, y);
	// Make a copy of the sprite to be the shadow's sprite
	auto& spriteComponent = registry.assign<SpriteComponent>(shadow, std::make_shared<sf::Sprite>(sprite));
	//TODO: make these shadow trail numbers some constants
	spriteComponent.setEffectAnimation(std::make_unique<FadeAwaySEA>(spriteComponent.getSprite(), 0, 0.75f, shadowLifespan));
	registry.assign<DespawnComponent>(shadow, shadowLifespan);
}

int SpawnShadowTrailCommand::getEntitiesQueuedCount() {
	return 1;
}