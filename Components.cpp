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
#include "Item.h"
#include "Player.h"
#include "CollisionSystem.h"
#include <math.h>
#include <tuple>

// Used to account for float inaccuracies
const float sigma = 0.00001f;

void MovementPathComponent::update(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, PositionComponent& entityPosition, float deltaTime) {
	time += deltaTime;
	// While loop for actions with lifespan of 0 like DetachFromParent 
	while (currentActionsIndex < actions.size() && time >= path->getLifespan()) {
		// Set this entity's position to last point of the ending MovablePoint to prevent inaccuracies from building up in updates
		if (useReferenceEntity) {
			auto& pos = registry.get<PositionComponent>(referenceEntity);
			entityPosition.setPosition(path->compute(sf::Vector2f(pos.getX(), pos.getY()), path->getLifespan()));
		} else {
			entityPosition.setPosition(path->compute(sf::Vector2f(0, 0), path->getLifespan()));
		}
		
		time -= path->getLifespan();
		previousPaths.push_back(path);
		path = actions[currentActionsIndex]->execute(queue, registry, entity, time);
		currentActionsIndex++;
	}
	if (useReferenceEntity) {
		auto& pos = registry.get<PositionComponent>(referenceEntity);
		entityPosition.setPosition(path->compute(sf::Vector2f(pos.getX(), pos.getY()), time));
	} else {
		entityPosition.setPosition(path->compute(sf::Vector2f(0, 0), time));
	}
}

sf::Vector2f MovementPathComponent::getPreviousPosition(entt::DefaultRegistry & registry, float secondsAgo) const {
	// This function assumes that if a reference entity has no MovementPathComponent, it has stayed in the same position its entire lifespan

	float curTime = time - secondsAgo;
	if (curTime >= 0) {
		// This entity was on the same path [secondsAgo] seconds ago

		if (useReferenceEntity) {
			if (registry.has<MovementPathComponent>(referenceEntity)) {
				auto pos = registry.get<MovementPathComponent>(referenceEntity).getPreviousPosition(registry, secondsAgo);
				return path->compute(sf::Vector2f(pos.x, pos.y), curTime);
			} else {
				auto& pos = registry.get<PositionComponent>(referenceEntity);
				return path->compute(sf::Vector2f(pos.getX(), pos.getY()), curTime);
			}
		} else {
			return path->compute(sf::Vector2f(0, 0), curTime);
		}
	} else {
		int curPathIndex = previousPaths.size();
		while (curTime < 0) {
			assert(curPathIndex - 1 >= 0 && "Somehow looking back in the past too far");
			curTime += previousPaths[curPathIndex - 1]->getLifespan();
			curPathIndex--;
		}

		if (useReferenceEntity) {
			if (registry.has<MovementPathComponent>(referenceEntity)) {
				auto pos = registry.get<MovementPathComponent>(referenceEntity).getPreviousPosition(registry, secondsAgo);
				return previousPaths[curPathIndex]->compute(sf::Vector2f(pos.x, pos.y), curTime);
			} else {
				auto& pos = registry.get<PositionComponent>(referenceEntity);
				return previousPaths[curPathIndex]->compute(sf::Vector2f(pos.getX(), pos.getY()), curTime);
			}
		} else {
			return previousPaths[curPathIndex]->compute(sf::Vector2f(0, 0), curTime);
		}
	}
}

void MovementPathComponent::setPath(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, PositionComponent& entityPosition, std::shared_ptr<MovablePoint> newPath, float timeLag) {
	// Put old path into history

	// Since the old path ended unexpectedly, change its lifespan
	path->setLifespan(time - timeLag);
	previousPaths.push_back(path);
	path = newPath;

	time = timeLag;
	update(queue, registry, entity, entityPosition, 0);
}

void MovementPathComponent::initialSpawn(entt::DefaultRegistry& registry, uint32_t entity, std::shared_ptr<EMPSpawnType> spawnType, std::vector<std::shared_ptr<EMPAction>>& actions) {
	auto spawnInfo = spawnType->getSpawnInfo(registry, entity, time);
	useReferenceEntity = spawnInfo.useReferenceEntity;
	referenceEntity = spawnInfo.referenceEntity;

	path = std::make_shared<StationaryMP>(spawnInfo.position, 0);
}

EnemyComponent::EnemyComponent(std::shared_ptr<EditorEnemy> enemyData, EnemySpawnInfo spawnInfo, int enemyID) : enemyData(enemyData), spawnInfo(spawnInfo), enemyID(enemyID) {}

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

			currentPhase = levelPack.getEnemyPhase(std::get<1>(nextPhaseData));
			currentAttackPattern = nullptr;

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
	while (currentPhase) {
		auto nextAttackPattern = currentPhase->getAttackPatternData(currentAttackPatternIndex + 1);
		// Check if condition for next attack pattern is satisfied
		if (timeSincePhase >= nextAttackPattern.first) {
			timeSinceAttackPattern = timeSincePhase - nextAttackPattern.first;

			currentAttackPatternIndex++;
			currentAttackIndex = -1;

			currentAttackPattern = levelPack.getAttackPattern(nextAttackPattern.second);

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
		if (timeSinceAttackPattern >= nextAttack.first) {
			currentAttackIndex++;
			levelPack.getAttack(nextAttack.second)->executeAsEnemy(queue, spriteLoader, registry, entity, timeSinceAttackPattern - nextAttack.first, currentAttackPattern->getID(), enemyID, currentPhase->getID());
		} else {
			break;
		}
	}
}

LevelManagerTag::LevelManagerTag(LevelPack* levelPack, std::shared_ptr<Level> level) : levelPack(levelPack), level(level) {
}

void LevelManagerTag::update(EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry& registry, float deltaTime) {
	timeSinceStartOfLevel += deltaTime;
	timeSinceLastEnemySpawn += deltaTime;

	while (currentEnemyGroupSpawnIndex + 1 < level->getEnemyGroupsCount()) {
		if (level->conditionSatisfied(currentEnemyGroupSpawnIndex + 1, registry)) {
			for (EnemySpawnInfo info : level->getEnemyGroupSpawnInfo(currentEnemyGroupSpawnIndex + 1)) {
				info.spawnEnemy(spriteLoader, *levelPack, registry, queue);
			}

			currentEnemyGroupSpawnIndex++;
		} else {
			break;
		}
	}
}

void SpriteComponent::update(float deltaTime) {
	if (animation != nullptr) {
		auto newSprite = animation->update(deltaTime);
		if (newSprite == nullptr) {
			// Animation is finished, so revert back to original sprite
			updateSprite(originalSprite);
		} else {
			updateSprite(newSprite);
		}
	}
	if (sprite) {
		if (effectAnimation != nullptr) {
			effectAnimation->update(deltaTime);
		}

		// Rotate sprite
		if (rotationType == ROTATE_WITH_MOVEMENT) {
			// Negative because SFML uses clockwise rotation
			sprite->setRotation(-rotationAngle * 180.0 / PI);
		} else if (rotationType == LOCK_ROTATION) {
			// Do nothing
		} else if (rotationType == LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT) {
			// Flip across y-axis if facing left
			auto curScale = sprite->getScale();
			if (rotationAngle < -PI / 2.0f - sigma || rotationAngle > PI / 2.0f + sigma) {
				lastFacedRight = false;
				if (curScale.x > 0) {
					sprite->setScale(-1.0f * curScale.x, curScale.y);
				}
			} else if (rotationAngle > -PI / 2.0f + sigma && rotationAngle < PI / 2.0f - sigma) {
				lastFacedRight = true;
				if (curScale.x < 0) {
					sprite->setScale(-1.0f * curScale.x, curScale.y);
				}
			} else if ((lastFacedRight && curScale.x < 0) || (!lastFacedRight && curScale.x > 0)) {
				sprite->setScale(-1.0f * curScale.x, curScale.y);
			}
			// Do nothing (maintain last values) if angle is a perfect 90 or -90 degree angle
		}
	}
}

EMPSpawnerComponent::EMPSpawnerComponent(std::vector<std::shared_ptr<EditorMovablePoint>> emps, uint32_t parent, int attackID, int attackPatternID, int enemyID, int enemyPhaseID, bool playAttackAnimation) : parent(parent), attackID(attackID), attackPatternID(attackPatternID), enemyID(enemyID), enemyPhaseID(enemyPhaseID), playAttackAnimation(playAttackAnimation) {
	for (auto emp : emps) {
		this->emps.push(emp);
	}
}

EMPSpawnerComponent::EMPSpawnerComponent(std::vector<std::shared_ptr<EditorMovablePoint>> emps, uint32_t parent, int attackID, int attackPatternID, bool playAttackAnimation) : parent(parent), attackID(attackID), attackPatternID(attackPatternID), playAttackAnimation(playAttackAnimation) {
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
				queue.pushBack(std::make_unique<EMPSpawnFromEnemyCommand>(registry, spriteLoader, emps.front(), parent, time - t, attackID, attackPatternID, enemyID, enemyPhaseID, playAttackAnimation));
				emps.pop();
			} else {
				break;
			}
		}
	} else {
		while (!emps.empty()) {
			float t = emps.front()->getSpawnType()->getTime();
			if (time >= t) {
				queue.pushBack(std::make_unique<EMPSpawnFromPlayerCommand>(registry, spriteLoader, emps.front(), parent, time - t, attackID, attackPatternID, playAttackAnimation));
				emps.pop();
			} else {
				break;
			}
		}
	}
}

void AnimatableSetComponent::update(SpriteLoader& spriteLoader, float x, float y, SpriteComponent & spriteComponent, float deltaTime) {
	if (deltaTime == 0) {
		changeState(queuedState, spriteLoader, spriteComponent);
	} else {
		if ((x == lastX && y == lastY) || !firstUpdateHasBeenCalled) {
			changeState(IDLE, spriteLoader, spriteComponent);
		} else {
			changeState(MOVEMENT, spriteLoader, spriteComponent);
		}

		lastX = x;
		lastY = y;
		firstUpdateHasBeenCalled = true;
	}
}

void AnimatableSetComponent::changeState(int newState, SpriteLoader& spriteLoader, SpriteComponent & spriteComponent) {
	Animatable newAnimatable;
	bool changeState = false;
	bool loopNewAnimatable = false;
	if (newState == IDLE && (currentState == MOVEMENT || (currentState == ATTACK && spriteComponent.animationIsDone()) || currentState == -1)) {
		// Play idle animatable
		newAnimatable = animatableSet.getIdleAnimatable();
		loopNewAnimatable = true;
		changeState = true;
	} else if (newState == MOVEMENT && (currentState == IDLE || (currentState == ATTACK && spriteComponent.animationIsDone()) || currentState == -1)) {
		// Play movement animatable
		newAnimatable = animatableSet.getMovementAnimatable();
		loopNewAnimatable = true;
		changeState = true;
	} else if (newState == ATTACK) {
		// Play attack animatable
		newAnimatable = animatableSet.getAttackAnimatable();
		loopNewAnimatable = false;
		changeState = true;
	}
	queuedState = newState;

	if (changeState) {
		spriteComponent.setAnimatable(spriteLoader, newAnimatable, loopNewAnimatable);
		currentState = newState;
	}
}

PlayerTag::PlayerTag(entt::DefaultRegistry& registry, const LevelPack& levelPack, uint32_t self, float speed, float focusedSpeed, float invulnerabilityTime, const std::vector<PlayerPowerTier> powerTiers) : speed(speed), focusedSpeed(focusedSpeed), invulnerabilityTime(invulnerabilityTime), powerTiers(powerTiers) {
	for (int i = 0; i < powerTiers.size(); i++) {
		// Load all the attack patterns
		attackPatterns.push_back(levelPack.getAttackPattern(powerTiers[i].getAttackPatternID()));
		focusedAttackPatterns.push_back(levelPack.getAttackPattern(powerTiers[i].getFocusedAttackPatternID()));

		// Calculate attack pattern total times
		attackPatternTotalTimes.push_back(attackPatterns[i]->getAttackData(attackPatterns[i]->getAttacksCount() - 1).first + powerTiers[i].getAttackPatternLoopDelay());
		focusedAttackPatternTotalTimes.push_back(focusedAttackPatterns[i]->getAttackData(focusedAttackPatterns[i]->getAttacksCount() - 1).first + powerTiers[i].getFocusedAttackPatternLoopDelay());
	}

	currentPowerTierIndex = 0;
	registry.get<AnimatableSetComponent>(self).setAnimatableSet(powerTiers[currentPowerTierIndex].getAnimatableSet());
}

void PlayerTag::increasePower(entt::DefaultRegistry& registry, uint32_t self, int power) {
	if (currentPower + power >= POWER_PER_POWER_TIER) {
		if (currentPowerTierIndex + 1 < powerTiers.size()) {
			// Power tier up

			currentPowerTierIndex++;
			currentPower += power - POWER_PER_POWER_TIER;
			increasedPowerTier = true;

			// Update this entity's EntityAnimatableSet
			registry.get<AnimatableSetComponent>(self).setAnimatableSet(powerTiers[currentPowerTierIndex].getAnimatableSet());
		} else {
			// Already reached power cap

			currentPower += power;
			// Increase points
			registry.get<LevelManagerTag>().addPoints(POINTS_PER_EXTRA_POWER * (currentPower - POWER_PER_POWER_TIER));
			currentPower = POWER_PER_POWER_PACK;
		}
	} else {
		currentPower += power;
	}
}

void CollectibleComponent::update(EntityCreationQueue& queue, entt::DefaultRegistry & registry, uint32_t entity, const PositionComponent& entityPos, const HitboxComponent& entityHitbox) {
	if (!activated && collides(entityPos, entityHitbox, registry.get<PositionComponent>(registry.attachee<PlayerTag>()), registry.get<HitboxComponent>(registry.attachee<PlayerTag>()), activationRadius)) {
		// Item is activated, so begin moving towards the player

		auto speed = std::make_shared<PiecewiseContinuousTFV>();
		// Speed starts off quickly at 250 and slows to 150 by t=2
		speed->insertSegment(0, std::make_pair(2.0f, std::make_shared<DampenedEndTFV>(250.0f, 150.0f, 2.0f, 2)));
		// Speed then maintains a constant 150 forever
		speed->insertSegment(1, std::make_pair(std::numeric_limits<float>::max() - 3, std::make_shared<ConstantTFV>(150.0f)));
		auto newPath = std::make_shared<HomingMP>(std::numeric_limits<float>::max(), speed, std::make_shared<ConstantTFV>(0.03f), entity, registry.attachee<PlayerTag>(), registry);
		registry.get<MovementPathComponent>(entity).setPath(queue, registry, entity, registry.get<PositionComponent>(entity), newPath, 0);
		// Make sure item can't despawn
		registry.get<DespawnComponent>(entity).setMaxTime(std::numeric_limits<float>::max());
		activated = true;
	}
	if (activated && collides(entityPos, entityHitbox, registry.get<PositionComponent>(registry.attachee<PlayerTag>()), registry.get<HitboxComponent>(registry.attachee<PlayerTag>()), 0)) {
		// Player touched the item

		// Execute the item's onPlayerContact
		item->onPlayerContact(registry, registry.attachee<PlayerTag>());

		// Despawn self
		if (registry.has<DespawnComponent>(entity)) {
			registry.get<DespawnComponent>(entity).setMaxTime(0);
		} else {
			registry.assign<DespawnComponent>(entity, 0);
		}
	}
}

void HitboxComponent::rotate(float angle) {
	if (unrotatedX == unrotatedY == 0) return;

	if (rotationType == ROTATE_WITH_MOVEMENT) {
		float sin = std::sin(angle);
		float cos = std::cos(angle);
		x = unrotatedX * cos - unrotatedY * sin;
		y = unrotatedX * sin + unrotatedY * cos;
	} else if (rotationType == LOCK_ROTATION) {
		// Do nothing
	} else if (rotationType == LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT) {
		// Flip across y-axis if facing left
		if (angle < -PI / 2.0f - sigma || angle > PI / 2.0f + sigma) {
			y = -unrotatedY;
		} else if (angle > -PI / 2.0f + sigma && angle < PI / 2.0f - sigma) {
			y = unrotatedY;
		}
		// Do nothing (maintain last value) if angle is a perfect 90 or -90 degree angle
	}
}

BULLET_ON_COLLISION_ACTION EnemyBulletComponent::getOnCollisionAction() {
	return onCollisionAction;
}

BULLET_ON_COLLISION_ACTION PlayerBulletComponent::getOnCollisionAction() {
	return onCollisionAction;
}

LevelPack* LevelManagerTag::getLevelPack() {
	return levelPack;
}
