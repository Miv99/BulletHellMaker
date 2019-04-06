#include "EntityCreationQueue.h"
#include "EditorMovablePoint.h"
#include "EditorMovablePointAction.h"
#include "EditorMovablePointSpawnType.h"
#include "SpriteLoader.h"
#include "Enemy.h"
#include "EntityAnimatableSet.h"
#include "Constants.h"
#include <algorithm>
#include <iostream>
#include "MovablePoint.h"

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
	MPSpawnInformation spawnInfo = emp->getSpawnType()->getSpawnInfo(registry, entity, timeLag);

	// If the bullet's MP reference is the entity executing the attack, make sure the bullet despawns along with the entity
	if (spawnInfo.useReferenceEntity && spawnInfo.referenceEntity == entity) {
		registry.assign<DespawnComponent>(bullet, registry, entity, bullet);
	}

	// Spawn at 0, 0 because creation of the MovementPathComponent will just update the position anyway
	registry.assign<PositionComponent>(bullet, 0, 0);

	registry.assign<MovementPathComponent>(bullet, queue, bullet, registry, entity, emp->getSpawnType(), emp->getActions(), timeLag);

	// Add max time to DespawnComponent
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

	// If animatable name is not empty, this EMP is a bullet
	Animatable animatable = emp->getAnimatable();
	if (animatable.getAnimatableName() != "") {
		registry.assign<HitboxComponent>(bullet, emp->getRotationType(), emp->getHitboxRadius(), emp->getHitboxPosX(), emp->getHitboxPosY());
		registry.assign<SpriteComponent>(bullet, emp->getRotationType(), spriteLoader.getSprite(animatable.getAnimatableName(), animatable.getSpriteSheetName()));

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

	// Create the simple reference entity, since all entities must have one
	auto& lastPos = registry.get<PositionComponent>(bullet);
	queue.pushFront(std::make_unique<CreateMovementReferenceEntityCommand>(registry, bullet, timeLag, lastPos.getX(), lastPos.getY()));
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
	MPSpawnInformation spawnInfo = emp->getSpawnType()->getSpawnInfo(registry, entity, timeLag);

	// If the bullet's MP reference is the entity executing the attack, make sure the bullet despawns along with the entity
	if (spawnInfo.useReferenceEntity && spawnInfo.referenceEntity == entity) {
		registry.assign<DespawnComponent>(bullet, registry, entity, bullet);
	}

	// Spawn at 0, 0 because creation of the MovementPathComponent will just update the position anyway
	registry.assign<PositionComponent>(bullet, 0, 0);

	registry.assign<MovementPathComponent>(bullet, queue, bullet, registry, entity, emp->getSpawnType(), emp->getActions(), timeLag);

	// Add max time to DespawnComponent
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

	// If animatable name is not empty, this EMP is a bullet
	Animatable animatable = emp->getAnimatable();
	if (animatable.getAnimatableName() != "") {
		registry.assign<HitboxComponent>(bullet, emp->getRotationType(), emp->getHitboxRadius(), emp->getHitboxPosX(), emp->getHitboxPosY());
		registry.assign<SpriteComponent>(bullet, emp->getRotationType(), spriteLoader.getSprite(animatable.getAnimatableName(), animatable.getSpriteSheetName()));

		registry.assign<PlayerBulletComponent>(bullet, attackID, attackPatternID, emp->getDamage());
	}

	if (emp->getShadowTrailLifespan() > 0) {
		registry.assign<ShadowTrailComponent>(bullet, emp->getShadowTrailInterval(), emp->getShadowTrailLifespan());
	}

	if (emp->getChildren().size() > 0) {
		EMPSpawnerComponent& empSpawnerComponent = registry.assign<EMPSpawnerComponent>(bullet, emp->getChildren(), bullet, attackID, attackPatternID, playAttackAnimation);
		// Update in case there are any children that should be spawned instantly
		empSpawnerComponent.update(registry, spriteLoader, queue, 0);
	}

	// Create the simple reference entity, since all entities must have one
	auto& lastPos = registry.get<PositionComponent>(bullet);
	queue.pushFront(std::make_unique<CreateMovementReferenceEntityCommand>(registry, bullet, timeLag, lastPos.getX(), lastPos.getY()));
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
	registry.assign<DespawnComponent>(reference, registry, entity, reference);
	mpc.setReferenceEntity(reference);
	// Update position
	registry.get<PositionComponent>(entity).setPosition(mpc.getPath()->compute(sf::Vector2f(lastPosX, lastPosY), mpc.getTime()));
}

int EMPADetachFromParentCommand::getEntitiesQueuedCount() {
	return 1;
}

void CreateMovementReferenceEntityCommand::execute(EntityCreationQueue& queue) {
	auto& mpc = registry.get<MovementPathComponent>(entity);

	if (mpc.usesReferenceEntity()) {
		if (registry.has<SimpleEMPReferenceComponent>(mpc.getReferenceEntity())) {
			// If the entity's reference is a simple reference, just move it by updating its path

			auto reference = mpc.getReferenceEntity();
			auto& referenceMPC = registry.get<MovementPathComponent>(reference);

			// Calculate position of new reference
			sf::Vector2f brLastPos(0, 0);
			if (referenceMPC.usesReferenceEntity()) {
				auto baseReferencePos = registry.get<PositionComponent>(registry.get<MovementPathComponent>(reference).getReferenceEntity());
				brLastPos.x = baseReferencePos.getX();
				brLastPos.y = baseReferencePos.getY();
			}
			referenceMPC.setPath(queue, registry, reference, registry.get<PositionComponent>(reference), std::make_shared<StationaryMP>(sf::Vector2f(lastPosX - brLastPos.x, lastPosY - brLastPos.y), 0), timeLag);
		} else {
			// If the entity's reference is not a simple reference, create a new simple reference for it

			auto baseReference = mpc.getReferenceEntity();
			auto& brLastPos = registry.get<PositionComponent>(baseReference);

			// Make new reference entity
			auto reference = registry.create();
			registry.assign<SimpleEMPReferenceComponent>(reference);
			registry.assign<PositionComponent>(reference, lastPosX, lastPosY);
			registry.assign<MovementPathComponent>(reference, queue, reference, registry, baseReference, std::make_shared<EntityAttachedEMPSpawn>(0, lastPosX - brLastPos.getX(), lastPosY - brLastPos.getY()), std::vector<std::shared_ptr<EMPAction>>(), timeLag);
			// Reference despawns when the entity executing this action despawns
			registry.assign<DespawnComponent>(reference, registry, entity, reference);

			mpc.setReferenceEntity(reference);
		}
	} else {
		// If the entity has no reference, create a new simple reference for it

		// Make new reference entity
		auto reference = registry.create();
		registry.assign<SimpleEMPReferenceComponent>(reference);
		registry.assign<PositionComponent>(reference, lastPosX, lastPosY);
		registry.assign<MovementPathComponent>(reference, queue, reference, registry, reference, std::make_shared<SpecificGlobalEMPSpawn>(0, lastPosX, lastPosY), std::vector<std::shared_ptr<EMPAction>>(), timeLag);
		// Reference despawns when the entity executing this action despawns
		registry.assign<DespawnComponent>(reference, registry, entity, reference);

		mpc.setReferenceEntity(reference);
	}

	// Update position
	mpc.update(queue, registry, entity, registry.get<PositionComponent>(entity), 0);
}

int CreateMovementReferenceEntityCommand::getEntitiesQueuedCount() {
	return 1;
}

SpawnEnemyCommand::SpawnEnemyCommand(entt::DefaultRegistry & registry, SpriteLoader & spriteLoader, std::shared_ptr<EditorEnemy> enemyInfo, EnemySpawnInfo spawnInfo) : 
	EntityCreationCommand(registry), spriteLoader(spriteLoader), enemyInfo(enemyInfo), spawnInfo(spawnInfo) {
}

void SpawnEnemyCommand::execute(EntityCreationQueue & queue) {
	auto enemy = registry.create();
	registry.assign<PositionComponent>(enemy, spawnInfo.getX(), spawnInfo.getY());
	// EMPActions vector empty because it will be populated in EnemySystem when the enemy begins a phase
	registry.assign<MovementPathComponent>(enemy, queue, enemy, registry, enemy, std::make_shared<SpecificGlobalEMPSpawn>(0, spawnInfo.getX(), spawnInfo.getY()), std::vector<std::shared_ptr<EMPAction>>(), 0);
	registry.assign<HealthComponent>(enemy, enemyInfo->getHealth(), enemyInfo->getHealth());
	if (enemyInfo->getDespawnTime() > 0) {
		registry.assign<DespawnComponent>(enemy, enemyInfo->getDespawnTime());
	} else {
		// Assign empty DespawnComponent so that CollisionSystem can use it later without having to assign a DespawnComponent to the enemy
		registry.assign<DespawnComponent>(enemy);
	}
	registry.assign<HitboxComponent>(enemy, enemyInfo->getRotationType(), enemyInfo->getHitboxRadius(), enemyInfo->getHitboxPosX(), enemyInfo->getHitboxPosY());
	registry.assign<SpriteComponent>(enemy, enemyInfo->getRotationType());
	registry.assign<EnemyComponent>(enemy, enemyInfo, spawnInfo, enemyInfo->getID());
	registry.assign<AnimatableSetComponent>(enemy);
	registry.assign<ShadowTrailComponent>(enemy, 0, 0);

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
	auto& spriteComponent = registry.assign<SpriteComponent>(shadow, LOCK_ROTATION, std::make_shared<sf::Sprite>(sprite));
	spriteComponent.setEffectAnimation(std::make_unique<FadeAwaySEA>(spriteComponent.getSprite(), 0, SHADOW_TRAIL_MAX_OPACITY, shadowLifespan));
	registry.assign<DespawnComponent>(shadow, shadowLifespan);
}

int SpawnShadowTrailCommand::getEntitiesQueuedCount() {
	return 1;
}