#include "EntityCreationQueue.h"
#include "EditorMovablePoint.h"
#include "EditorMovablePointAction.h"
#include "SpriteLoader.h"
#include "Enemy.h"
#include "EntityAnimatableSet.h"
#include "Animatable.h"
#include "Constants.h"
#include <algorithm>
#include <iostream>
#include "MovablePoint.h"
#include "LevelPack.h"
#include <random>

EMPSpawnFromEnemyCommand::EMPSpawnFromEnemyCommand(entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, std::shared_ptr<EditorMovablePoint> emp, bool isMainEMP, uint32_t entity, float timeLag, int attackID, int attackPatternID, int enemyID, int enemyPhaseID, bool playAttackAnimation) :
	EntityCreationCommand(registry), spriteLoader(spriteLoader), emp(emp), isMainEMP(isMainEMP), playAttackAnimation(playAttackAnimation),
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

	// Make sure the bullet despawns along with its reference entity
	if (spawnInfo.useReferenceEntity) {
		registry.assign<DespawnComponent>(bullet, registry, spawnInfo.referenceEntity, bullet);
	} else {
		registry.assign<DespawnComponent>(bullet);
	}

	// Spawn at 0, 0 because creation of the MovementPathComponent will just update the position anyway
	registry.assign<PositionComponent>(bullet, 0, 0);

	auto& despawn = registry.get<DespawnComponent>(bullet);
	if (isMainEMP && emp->getHitboxRadius() < 0) {
		// The main EMP of an attack, if the EMP is not a bullet, despawns when all its children despawn

		despawn.setDespawnWhenNoChildren();
	} else {
		// Add max time to DespawnComponent despawn conditions
		if (emp->getDespawnTime() > 0) {
			despawn.setMaxTime(std::min(emp->getTotalPathTime(), emp->getDespawnTime()));
		} else {
			despawn.setMaxTime(emp->getTotalPathTime());
		}
	}

	if (emp->requiresBaseSprite()) {
		Animatable animatable = emp->getAnimatable();
		// Initialize with base sprite as the original sprite so that the base sprite will be the sprite that is reverted to when
		// the animation ends
		auto& sprite = registry.assign<SpriteComponent>(bullet, spriteLoader, emp->getBaseSprite(), true, ENEMY_BULLET_LAYER, registry.get<LevelManagerTag>().getTimeSinceStartOfLevel() - timeLag);
		sprite.setAnimatable(spriteLoader, animatable, emp->getLoopAnimation());
		if (emp->getIsBullet()) {
			registry.assign<HitboxComponent>(bullet, emp->getHitboxRadius(), sprite.getSprite());
		}
	}
	else {
		Animatable animatable = emp->getAnimatable();
		auto& sprite = registry.assign<SpriteComponent>(bullet, spriteLoader, animatable, emp->getLoopAnimation(), ENEMY_BULLET_LAYER, registry.get<LevelManagerTag>().getTimeSinceStartOfLevel() - timeLag);
		if (emp->getIsBullet()) {
			registry.assign<HitboxComponent>(bullet, emp->getHitboxRadius(), sprite.getSprite());
		}
	}

	if (emp->getIsBullet()) {
		registry.assign<EnemyBulletComponent>(bullet, attackID, attackPatternID, enemyID, enemyPhaseID, emp->getDamage(), emp->getOnCollisionAction(), emp->getPierceResetTime());
	}

	registry.assign<MovementPathComponent>(bullet, queue, bullet, registry, entity, emp->getSpawnType(), emp->getActions(), timeLag);

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

	// Play the sound associated with the EMP
	if (!emp->getSoundSettings().isDisabled()) {
		registry.get<LevelManagerTag>().getLevelPack()->playSound(emp->getSoundSettings());
	}
}

int EMPSpawnFromEnemyCommand::getEntitiesQueuedCount() {
	return emp->getTreeSize();
}


EMPSpawnFromNothingCommand::EMPSpawnFromNothingCommand(entt::DefaultRegistry & registry, SpriteLoader & spriteLoader, std::shared_ptr<EditorMovablePoint> emp, bool isMainEMP, float timeLag, int attackID, int attackPatternID) :
	EntityCreationCommand(registry), spriteLoader(spriteLoader), emp(emp), spawnInfoIsDefined(false), isMainEMP(isMainEMP), timeLag(timeLag), attackID(attackID), attackPatternID(attackPatternID) {
}

EMPSpawnFromNothingCommand::EMPSpawnFromNothingCommand(entt::DefaultRegistry & registry, SpriteLoader & spriteLoader, std::shared_ptr<EditorMovablePoint> emp, MPSpawnInformation spawnInfo, bool isMainEMP, float timeLag, int attackID, int attackPatternID) :
	EntityCreationCommand(registry), spriteLoader(spriteLoader), emp(emp), spawnInfo(spawnInfo), spawnInfoIsDefined(true), isMainEMP(isMainEMP), timeLag(timeLag), attackID(attackID), attackPatternID(attackPatternID) {
}

void EMPSpawnFromNothingCommand::execute(EntityCreationQueue & queue) {
	// Create the entity
	auto bullet = registry.create();
	if (!spawnInfoIsDefined) {
		spawnInfo = emp->getSpawnType()->getForcedDetachmentSpawnInfo(registry, timeLag);
	}

	// Make sure the bullet despawns along with its reference entity
	if (spawnInfo.useReferenceEntity) {
		registry.assign<DespawnComponent>(bullet, registry, spawnInfo.referenceEntity, bullet);
	} else {
		registry.assign<DespawnComponent>(bullet);
	}

	// Spawn at 0, 0 because creation of the MovementPathComponent will just update the position anyway
	registry.assign<PositionComponent>(bullet, 0, 0);

	auto& despawn = registry.get<DespawnComponent>(bullet);
	if (isMainEMP && emp->getHitboxRadius() < 0) {
		// The main EMP of an attack, if the EMP is not a bullet, despawns when all its children despawn

		despawn.setDespawnWhenNoChildren();
	} else {
		// Add max time to DespawnComponent despawn conditions
		if (emp->getDespawnTime() > 0) {
			despawn.setMaxTime(std::min(emp->getTotalPathTime(), emp->getDespawnTime()));
		} else {
			despawn.setMaxTime(emp->getTotalPathTime());
		}
	}

	if (emp->requiresBaseSprite()) {
		Animatable animatable = emp->getAnimatable();
		// Initialize with base sprite as the original sprite so that the base sprite will be the sprite that is reverted to when
		// the animation ends
		auto& sprite = registry.assign<SpriteComponent>(bullet, spriteLoader, emp->getBaseSprite(), true, ENEMY_BULLET_LAYER, registry.get<LevelManagerTag>().getTimeSinceStartOfLevel() - timeLag);
		sprite.setAnimatable(spriteLoader, animatable, emp->getLoopAnimation());
		if (emp->getIsBullet()) {
			registry.assign<HitboxComponent>(bullet, emp->getHitboxRadius(), sprite.getSprite());
		}
	} else {
		Animatable animatable = emp->getAnimatable();
		auto& sprite = registry.assign<SpriteComponent>(bullet, spriteLoader, animatable, emp->getLoopAnimation(), ENEMY_BULLET_LAYER, registry.get<LevelManagerTag>().getTimeSinceStartOfLevel() - timeLag);
		if (emp->getIsBullet()) {
			registry.assign<HitboxComponent>(bullet, emp->getHitboxRadius(), sprite.getSprite());
		}
	}

	if (emp->getIsBullet()) {
		registry.assign<EnemyBulletComponent>(bullet, attackID, attackPatternID, -1, -1, emp->getDamage(), emp->getOnCollisionAction(), emp->getPierceResetTime());
	}

	registry.assign<MovementPathComponent>(bullet, queue, bullet, registry, -1, spawnInfo, emp->getActions(), timeLag);

	if (emp->getShadowTrailLifespan() > 0) {
		registry.assign<ShadowTrailComponent>(bullet, emp->getShadowTrailInterval(), emp->getShadowTrailLifespan());
	}

	if (emp->getChildren().size() > 0) {
		EMPSpawnerComponent& empSpawnerComponent = registry.assign<EMPSpawnerComponent>(bullet, emp->getChildren(), bullet, attackID, attackPatternID, -1, -1, false);
		// Update in case there are any children that should be spawned instantly
		empSpawnerComponent.update(registry, spriteLoader, queue, 0);
	}

	// Create the simple reference entity, since all entities must have one
	auto& lastPos = registry.get<PositionComponent>(bullet);
	queue.pushFront(std::make_unique<CreateMovementReferenceEntityCommand>(registry, bullet, timeLag, lastPos.getX(), lastPos.getY()));

	// Play the sound associated with the EMP
	if (!emp->getSoundSettings().isDisabled()) {
		registry.get<LevelManagerTag>().getLevelPack()->playSound(emp->getSoundSettings());
	}
}

int EMPSpawnFromNothingCommand::getEntitiesQueuedCount() {
	return emp->getTreeSize();
}

EMPSpawnFromPlayerCommand::EMPSpawnFromPlayerCommand(entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, std::shared_ptr<EditorMovablePoint> emp, bool isMainEMP, uint32_t entity, float timeLag, int attackID, int attackPatternID, bool playAttackAnimation) :
	EntityCreationCommand(registry), spriteLoader(spriteLoader), emp(emp), isMainEMP(isMainEMP), playAttackAnimation(playAttackAnimation),
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

	// Make sure the bullet despawns along with its reference entity
	if (spawnInfo.useReferenceEntity) {
		registry.assign<DespawnComponent>(bullet, registry, spawnInfo.referenceEntity, bullet);
	} else {
		registry.assign<DespawnComponent>(bullet);
	}

	// Spawn at 0, 0 because creation of the MovementPathComponent will just update the position anyway
	registry.assign<PositionComponent>(bullet, 0, 0);

	auto& despawn = registry.get<DespawnComponent>(bullet);
	if (isMainEMP && emp->getHitboxRadius() < 0) {
		// The main EMP of an attack, if the EMP is not a bullet, despawns when all its children despawn

		despawn.setDespawnWhenNoChildren();
	} else {
		// Add max time to DespawnComponent despawn conditions
		if (emp->getDespawnTime() > 0) {
			despawn.setMaxTime(std::min(emp->getTotalPathTime(), emp->getDespawnTime()));
		} else {
			despawn.setMaxTime(emp->getTotalPathTime());
		}
	}

	if (emp->requiresBaseSprite()) {
		Animatable animatable = emp->getAnimatable();
		// Initialize with base sprite as the original sprite so that the base sprite will be the sprite that is reverted to when
		// the animation ends
		auto& sprite = registry.assign<SpriteComponent>(bullet, spriteLoader, emp->getBaseSprite(), true, PLAYER_BULLET_LAYER, registry.get<LevelManagerTag>().getTimeSinceStartOfLevel() - timeLag);
		sprite.setAnimatable(spriteLoader, animatable, emp->getLoopAnimation());
		if (emp->getIsBullet()) {
			registry.assign<HitboxComponent>(bullet, emp->getHitboxRadius(), sprite.getSprite());
		}
	}
	else {
		Animatable animatable = emp->getAnimatable();
		auto& sprite = registry.assign<SpriteComponent>(bullet, spriteLoader, animatable, emp->getLoopAnimation(), PLAYER_BULLET_LAYER, registry.get<LevelManagerTag>().getTimeSinceStartOfLevel() - timeLag);
		if (emp->getIsBullet()) {
			registry.assign<HitboxComponent>(bullet, emp->getHitboxRadius(), sprite.getSprite());
		}
	}

	if (emp->getIsBullet()) {
		registry.assign<PlayerBulletComponent>(bullet, attackID, attackPatternID, emp->getDamage(), emp->getOnCollisionAction(), emp->getPierceResetTime());
	}

	registry.assign<MovementPathComponent>(bullet, queue, bullet, registry, entity, emp->getSpawnType(), emp->getActions(), timeLag);

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

	// Play the sound associated with the EMP
	if (!emp->getSoundSettings().isDisabled()) {
		registry.get<LevelManagerTag>().getLevelPack()->playSound(emp->getSoundSettings());
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

SpawnEnemyCommand::SpawnEnemyCommand(entt::DefaultRegistry & registry, SpriteLoader & spriteLoader, std::shared_ptr<EditorEnemy> enemyInfo, std::shared_ptr<EnemySpawnInfo> spawnInfo) :
	EntityCreationCommand(registry), spriteLoader(spriteLoader), enemyInfo(enemyInfo), spawnInfo(spawnInfo) {
}

void SpawnEnemyCommand::execute(EntityCreationQueue & queue) {
	auto enemy = registry.create();
	registry.assign<PositionComponent>(enemy, spawnInfo->getX(), spawnInfo->getY());
	// EMPActions vector empty because it will be populated in EnemySystem when the enemy begins a phase
	registry.assign<MovementPathComponent>(enemy, queue, enemy, registry, enemy, std::make_shared<SpecificGlobalEMPSpawn>(0, spawnInfo->getX(), spawnInfo->getY()), std::vector<std::shared_ptr<EMPAction>>(), 0);
	registry.assign<HealthComponent>(enemy, enemyInfo->getHealth(), enemyInfo->getHealth());
	if (enemyInfo->getDespawnTime() > 0) {
		registry.assign<DespawnComponent>(enemy, enemyInfo->getDespawnTime());
	} else {
		// Assign empty DespawnComponent so that CollisionSystem can use it later without having to assign a DespawnComponent to the enemy
		registry.assign<DespawnComponent>(enemy);
	}
	// Hitbox temporarily at 0, 0 until an Animatable is assigned to the enemy on phase start
	registry.assign<HitboxComponent>(enemy, LOCK_ROTATION, enemyInfo->getHitboxRadius(), 0, 0);
	int layer = enemyInfo->getIsBoss() ? ENEMY_BOSS_LAYER : ENEMY_LAYER;
	registry.assign<SpriteComponent>(enemy, layer, registry.get<LevelManagerTag>().getTimeSinceStartOfLevel());
	registry.assign<EnemyComponent>(enemy, enemyInfo, spawnInfo, enemyInfo->getID());
	registry.assign<AnimatableSetComponent>(enemy);
	registry.assign<ShadowTrailComponent>(enemy, 0, 0);

	registry.get<LevelManagerTag>().onEnemySpawn(enemy);
}

int SpawnEnemyCommand::getEntitiesQueuedCount() {
	return 1;
}

void SpawnShadowTrailCommand::execute(EntityCreationQueue & queue) {
	auto shadow = registry.create();
	registry.assign<PositionComponent>(shadow, x, y);
	// Make a copy of the sprite to be the shadow's sprite
	auto& spriteComponent = registry.assign<SpriteComponent>(shadow, LOCK_ROTATION, std::make_shared<sf::Sprite>(sprite), SHADOW_LAYER, registry.get<LevelManagerTag>().getTimeSinceStartOfLevel());
	spriteComponent.setEffectAnimation(std::make_unique<FadeAwaySEA>(spriteComponent.getSprite(), 0, SHADOW_TRAIL_MAX_OPACITY, shadowLifespan));
	registry.assign<DespawnComponent>(shadow, shadowLifespan);
}

int SpawnShadowTrailCommand::getEntitiesQueuedCount() {
	return 1;
}

EMPDropItemCommand::EMPDropItemCommand(entt::DefaultRegistry & registry, SpriteLoader & spriteLoader, float x, float y, std::shared_ptr<Item> item, int amount) : 
	EntityCreationCommand(registry), spriteLoader(spriteLoader), x(x), y(y), amount(amount), item(item) {
}

void EMPDropItemCommand::execute(EntityCreationQueue & queue) {
	std::mt19937 eng;
	std::uniform_real_distribution<float> explosionTimeDistribution(1.0f, 2.0f);
	std::uniform_real_distribution<float> explosionDistanceDistribution(150.0f, 250.0f);
	std::uniform_real_distribution<float> explosionAngleDistribution(70.0f * PI/180.0f, 110.0f * PI/180.0f);

	float spriteSublayer = registry.get<LevelManagerTag>().getTimeSinceStartOfLevel();

	for (int i = 0; i < amount; i++) {
		uint32_t itemEntity = registry.create();
		registry.assign<CollectibleComponent>(itemEntity, item, item->getActivationRadius());
		registry.assign<DespawnComponent>(itemEntity, ITEM_DESPAWN_TIME);
		auto& sprite = registry.assign<SpriteComponent>(itemEntity, spriteLoader, item->getAnimatable(), true, ITEM_LAYER, spriteSublayer);
		registry.assign<HitboxComponent>(itemEntity, item->getHitboxRadius(), sprite.getSprite());
		registry.assign<PositionComponent>(itemEntity, x, y);

		// Movement path mimics an explosion upwards (70-110 degrees) and then dropping down
		std::vector<std::shared_ptr<EMPAction>> actions;
		// Explosion
		float explosionTime = explosionTimeDistribution(eng);
		std::shared_ptr<TFV> explosionDistance = std::make_shared<DampenedEndTFV>(0, explosionDistanceDistribution(eng), explosionTime, 10);
		std::shared_ptr<TFV> explosionAngle = std::make_shared<ConstantTFV>(explosionAngleDistribution(eng));
		actions.push_back(std::make_shared<MoveCustomPolarEMPA>(explosionDistance, explosionAngle, explosionTime));
		// Drop down
		std::shared_ptr<TFV> dropDistance = std::make_shared<DampenedStartTFV>(0, MAP_HEIGHT + 250 + sprite.getSprite()->getOrigin().y, ITEM_DESPAWN_TIME - explosionTime, 10);
		std::shared_ptr<TFV> dropAngle = std::make_shared<ConstantTFV>(3.0f * PI/2.0f);
		actions.push_back(std::make_shared<MoveCustomPolarEMPA>(dropDistance, dropAngle, ITEM_DESPAWN_TIME));
		registry.assign<MovementPathComponent>(itemEntity, queue, itemEntity, registry, NULL, std::make_shared<SpecificGlobalEMPSpawn>(0, x, y), actions, 0);
	}
}

int EMPDropItemCommand::getEntitiesQueuedCount() {
	return amount;
}

ParticleExplosionCommand::ParticleExplosionCommand(entt::DefaultRegistry & registry, SpriteLoader& spriteLoader, float sourceX, float sourceY, Animatable animatable, bool loopAnimatable,
	ParticleExplosionDeathAction::PARTICLE_EFFECT effect, sf::Color color, int minParticles, int maxParticles, float minDistance, float maxDistance, float minLifespan, float maxLifespan) :
	EntityCreationCommand(registry), spriteLoader(spriteLoader), sourceX(sourceX), sourceY(sourceY), animatable(animatable), loopAnimatable(loopAnimatable), effect(effect),
	color(color), minParticles(minParticles), maxParticles(maxParticles), minDistance(minDistance), maxDistance(maxDistance), minLifespan(minLifespan), maxLifespan(maxLifespan) {
}

void ParticleExplosionCommand::execute(EntityCreationQueue & queue) {
	std::mt19937 eng;
	std::uniform_int_distribution<int> particlesCount(minParticles, maxParticles);
	std::uniform_real_distribution<float> lifespan(minLifespan, maxLifespan);
	std::uniform_real_distribution<float> distance(minDistance, maxDistance);
	std::uniform_real_distribution<float> angle(0.0f, PI2);

	float spriteSublayer = registry.get<LevelManagerTag>().getTimeSinceStartOfLevel();

	for (int i = 0; i < particlesCount(eng); i++) {
		float particleLifespan = lifespan(eng);

		uint32_t particle = registry.create();
		registry.assign<DespawnComponent>(particle, particleLifespan);
		registry.assign<PositionComponent>(particle, sourceX, sourceY);
		auto& sprite = registry.assign<SpriteComponent>(particle, spriteLoader, animatable, loopAnimatable, PARTICLE_LAYER, spriteSublayer);
		// Overwrite sprite sheet entry's color
		sprite.getSprite()->setColor(color);

		std::vector<std::shared_ptr<EMPAction>> path = { std::make_shared<MoveCustomPolarEMPA>(std::make_shared<LinearTFV>(0, distance(eng), particleLifespan), std::make_shared<ConstantTFV>(angle(eng)), particleLifespan) };
		registry.assign<MovementPathComponent>(particle, queue, particle, registry, particle, std::make_shared<SpecificGlobalEMPSpawn>(0, sourceX, sourceY), path, 0);

		if (effect == ParticleExplosionDeathAction::PARTICLE_EFFECT::NONE) {
			// Do nothing
		} else if (effect == ParticleExplosionDeathAction::PARTICLE_EFFECT::FADE_AWAY) {
			sprite.setEffectAnimation(std::make_unique<FadeAwaySEA>(sprite.getSprite(), 0, color.a/255.0f, particleLifespan));
		} else if (effect == ParticleExplosionDeathAction::PARTICLE_EFFECT::SHRINK) {
			sprite.setEffectAnimation(std::make_unique<ChangeSizeSEA>(sprite.getSprite(), 1, 0, particleLifespan));
		}
	}
}

int ParticleExplosionCommand::getEntitiesQueuedCount() {
	// Return the max, since it's the worst-case scenario and a too-high estimate won't affect performance
	return maxParticles;
}

PlayDeathAnimatableCommand::PlayDeathAnimatableCommand(entt::DefaultRegistry & registry, SpriteLoader & spriteLoader, uint32_t dyingEntity, Animatable animatable, PlayAnimatableDeathAction::DEATH_ANIMATION_EFFECT effect, float duration) :
EntityCreationCommand(registry), spriteLoader(spriteLoader), dyingEntity(dyingEntity), animatable(animatable), effect(effect), duration(duration) {
}

void PlayDeathAnimatableCommand::execute(EntityCreationQueue & queue) {
	uint32_t newEntity = registry.create();
	auto& inheritedSpriteComponent = registry.get<SpriteComponent>(dyingEntity);

	auto& dyingSprite = registry.get<SpriteComponent>(dyingEntity);
	auto& spriteComponent = registry.assign<SpriteComponent>(newEntity, spriteLoader, animatable, false, dyingSprite.getRenderLayer(), dyingSprite.getSubLayer());
	if (animatable.isSprite()) {
		registry.assign<DespawnComponent>(newEntity, duration);
	} else {
		registry.assign<DespawnComponent>(newEntity, spriteLoader.getAnimation(animatable.getAnimatableName(), animatable.getSpriteSheetName(), false)->getTotalDuration());
	}
	spriteComponent.rotate(inheritedSpriteComponent.getInheritedRotationAngle());
	if (effect == PlayAnimatableDeathAction::DEATH_ANIMATION_EFFECT::NONE) {
		// Do nothing
	} else if (effect == PlayAnimatableDeathAction::DEATH_ANIMATION_EFFECT::SHRINK) {
		spriteComponent.setEffectAnimation(std::make_unique<ChangeSizeSEA>(spriteComponent.getSprite(), 0.0f, 1.0f, duration));
	} else if (effect == PlayAnimatableDeathAction::DEATH_ANIMATION_EFFECT::FADE_AWAY) {
		spriteComponent.setEffectAnimation(std::make_unique<FadeAwaySEA>(spriteComponent.getSprite(), 0.0f, spriteComponent.getSprite()->getColor().a, duration));
	}

	auto& oldPos = registry.get<PositionComponent>(dyingEntity);
	registry.assign<PositionComponent>(newEntity, oldPos.getX(), oldPos.getY());
}

int PlayDeathAnimatableCommand::getEntitiesQueuedCount() {
	return 1;
}