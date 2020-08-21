#include <Game/Components/Components.h>

#include <math.h>
#include <tuple>

#include <DataStructs/MovablePoint.h>
#include <LevelPack/LevelPack.h>
#include <LevelPack/EditorMovablePointAction.h>
#include <LevelPack/AttackPattern.h>
#include <LevelPack/Enemy.h>
#include <LevelPack/EnemyPhase.h>
#include <LevelPack/Attack.h>
#include <LevelPack/EditorMovablePointSpawnType.h>
#include <LevelPack/EnemySpawn.h>
#include <Game/EntityCreationQueue.h>
#include <LevelPack/EditorMovablePoint.h>
#include <LevelPack/Item.h>
#include <LevelPack/Player.h>
#include <Game/Systems/CollisionSystem.h>
#include <LevelPack/Level.h>
#include <Game/GameInstance.h>
#include <LevelPack/LevelEvent.h>

// Used to account for float inaccuracies
const float sigma = 0.00001f;

MovementPathComponent::MovementPathComponent(EntityCreationQueue & queue, uint32_t self, entt::DefaultRegistry & registry, uint32_t entity, MPSpawnInformation spawnInfo, std::vector<std::shared_ptr<EMPAction>> actions, float initialTime) : actions(actions), time(initialTime) {
	initialSpawn(registry, entity, spawnInfo, actions);
	update(queue, registry, self, registry.get<PositionComponent>(self), 0);
}

void MovementPathComponent::update(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, PositionComponent& entityPosition, float deltaTime) {
	time += deltaTime;
	sf::Vector2f tempReference(0, 0);
	// While loop for actions with lifespan of 0 like DetachFromParent 
	while (currentActionsIndex < actions.size() && time >= path->getLifespan()) {
		// Set this entity's position to last point of the ending MovablePoint to prevent inaccuracies from building up in updates
		if (useReferenceEntity) {
			auto& pos = registry.get<PositionComponent>(referenceEntity);
			entityPosition.setPosition(path->compute(sf::Vector2f(pos.getX(), pos.getY()), path->getLifespan()));
		} else {
			entityPosition.setPosition(path->compute(tempReference, path->getLifespan()));
		}
		
		time -= path->getLifespan();
		previousPaths.push_back(path);
		path = actions[currentActionsIndex]->execute(queue, registry, entity, time);
		currentActionsIndex++;

		tempReference.x = entityPosition.getX();
		tempReference.y = entityPosition.getY();
	}
	if (time <= path->getLifespan()) {
		if (useReferenceEntity) {
			auto& pos = registry.get<PositionComponent>(referenceEntity);
			entityPosition.setPosition(path->compute(sf::Vector2f(pos.getX(), pos.getY()), time));
		} else {
			entityPosition.setPosition(path->compute(tempReference, time));
		}
	} else {
		// The path's lifespan has been exceeded and there are no more paths to execute, so just stay at the last position on the path, relative to the reference entity

		if (useReferenceEntity) {
			auto& pos = registry.get<PositionComponent>(referenceEntity);
			entityPosition.setPosition(path->compute(sf::Vector2f(pos.getX(), pos.getY()), path->getLifespan()));
		} else {
			entityPosition.setPosition(path->compute(tempReference, path->getLifespan()));
		}
	}
}

sf::Vector2f MovementPathComponent::getPreviousPosition(entt::DefaultRegistry & registry, float secondsAgo) const {
	// This function assumes that if a reference entity has no MovementPathComponent, it has stayed in the same position its entire lifespan

	float curTime = time - secondsAgo;
	// Account for float inaccuracies
	if (curTime >= -0.0001f) {
		// This entity was on the same path [secondsAgo] seconds ago

		// curTime was probably supposed to be 0 but ended up being < 0 from float inaccuracies, so set it to 0
		if (curTime < 0) {
			curTime = 0;
		}

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

void MovementPathComponent::initialSpawn(entt::DefaultRegistry & registry, uint32_t entity, MPSpawnInformation spawnInfo, std::vector<std::shared_ptr<EMPAction>>& actions) {
	useReferenceEntity = spawnInfo.useReferenceEntity;
	referenceEntity = spawnInfo.referenceEntity;

	path = std::make_shared<StationaryMP>(spawnInfo.position, 0);
}

EnemyComponent::EnemyComponent(std::shared_ptr<EditorEnemy> enemyData, std::shared_ptr<EnemySpawnInfo> spawnInfo, int enemyID) : enemyData(enemyData), spawnInfo(spawnInfo), enemyID(enemyID) {}

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

LevelManagerTag::LevelManagerTag(LevelPack* levelPack, std::shared_ptr<Level> level, GameInstance* gameInstance) : levelPack(levelPack), level(level), gameInstance(gameInstance) {
}

LevelManagerTag::LevelManagerTag(LevelPack* levelPack, std::shared_ptr<Level> level) : levelPack(levelPack), level(level) {
}

void LevelManagerTag::showDialogue(ShowDialogueLevelEvent* dialogueEvent) {
	// Only show dialogue when playing a game using GameInstance
	if (gameInstance) {
		gameInstance->showDialogue(dialogueEvent);
	}
}

void LevelManagerTag::update(EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry& registry, float deltaTime) {
	timeSinceStartOfLevel += deltaTime;
	timeSinceLastEnemySpawn += deltaTime;

	while (currentLevelEventsIndex + 1 < level->getEventsCount()) {
		if (level->conditionSatisfied(currentLevelEventsIndex + 1, registry)) {
			level->executeEvent(currentLevelEventsIndex + 1, spriteLoader, *levelPack, registry, queue);
			currentLevelEventsIndex++;
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

bool SpriteComponent::animationIsDone() const {
	if (animation) {
		return animation->isDone();
	}
	return true;
}

EMPSpawnerComponent::EMPSpawnerComponent(std::vector<std::shared_ptr<EditorMovablePoint>> emps, uint32_t parent, int attackID, int attackPatternID, int enemyID, int enemyPhaseID, bool playAttackAnimation) : parent(parent), attackID(attackID), attackPatternID(attackPatternID), enemyID(enemyID), enemyPhaseID(enemyPhaseID), playAttackAnimation(playAttackAnimation), isEnemyBulletSpawner(true) {
	spawnedBulletType = 0;
	for (auto emp : emps) {
		this->emps.push(emp);
	}
}

EMPSpawnerComponent::EMPSpawnerComponent(std::vector<std::shared_ptr<EditorMovablePoint>> emps, uint32_t parent, int attackID, int attackPatternID) {
	spawnedBulletType = 1;
	for (auto emp : emps) {
		this->emps.push(emp);
	}
}

EMPSpawnerComponent::EMPSpawnerComponent(std::vector<std::shared_ptr<EditorMovablePoint>> emps, uint32_t parent, int attackID, int attackPatternID, bool playAttackAnimation) : parent(parent), attackID(attackID), attackPatternID(attackPatternID), playAttackAnimation(playAttackAnimation), isEnemyBulletSpawner(false) {
	spawnedBulletType = 2;
	for (auto emp : emps) {
		this->emps.push(emp);
	}
}

void EMPSpawnerComponent::update(entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, EntityCreationQueue& queue, float deltaTime) {
	time += deltaTime;

	if (spawnedBulletType == 0) {
		while (!emps.empty()) {
			float t = emps.front()->getSpawnType()->getTime();
			if (time >= t) {
				queue.pushBack(std::make_unique<EMPSpawnFromEnemyCommand>(registry, spriteLoader, emps.front(), false, parent, time - t, attackID, attackPatternID, enemyID, enemyPhaseID, playAttackAnimation));
				emps.pop();
			} else {
				break;
			}
		}
	} else if (spawnedBulletType == 2) {
		while (!emps.empty()) {
			float t = emps.front()->getSpawnType()->getTime();
			if (time >= t) {
				queue.pushBack(std::make_unique<EMPSpawnFromPlayerCommand>(registry, spriteLoader, emps.front(), false, parent, time - t, attackID, attackPatternID, playAttackAnimation));
				emps.pop();
			} else {
				break;
			}
		}
	} else {
		while (!emps.empty()) {
			float t = emps.front()->getSpawnType()->getTime();
			if (time >= t) {
				queue.pushBack(std::make_unique<EMPSpawnFromNothingCommand>(registry, spriteLoader, emps.front(), false, time - t, attackID, attackPatternID));
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

PlayerTag::PlayerTag(entt::DefaultRegistry& registry, const LevelPack& levelPack, uint32_t self, float speed, float focusedSpeed, float invulnerabilityTime, std::vector<std::shared_ptr<PlayerPowerTier>> powerTiers,
	SoundSettings hurtSound, SoundSettings deathSound, int initialBombs, int maxBombs, float bombInvincibilityTime) :
	speed(speed), focusedSpeed(focusedSpeed), invulnerabilityTime(invulnerabilityTime), powerTiers(powerTiers), hurtSound(hurtSound), deathSound(deathSound), bombAttackPattern(bombAttackPattern), 
	bombs(initialBombs), maxBombs(maxBombs), bombInvincibilityTime(bombInvincibilityTime) {
	for (int i = 0; i < powerTiers.size(); i++) {
		bombCooldowns.push_back(powerTiers[i]->getBombCooldown());

		// Load all the attack patterns
		attackPatterns.push_back(levelPack.getGameplayAttackPattern(powerTiers[i]->getAttackPatternID(), powerTiers[i]->getCompiledAttackPatternSymbolsDefiner()));
		focusedAttackPatterns.push_back(levelPack.getGameplayAttackPattern(powerTiers[i]->getFocusedAttackPatternID(), powerTiers[i]->getCompiledAttackPatternSymbolsDefiner()));
		bombAttackPatterns.push_back(levelPack.getGameplayAttackPattern(powerTiers[i]->getBombAttackPatternID(), powerTiers[i]->getCompiledAttackPatternSymbolsDefiner()));

		// Calculate attack pattern total times
		if (attackPatterns[i]->getAttacksCount() > 0) {
			attackPatternTotalTimes.push_back(std::get<0>(attackPatterns[i]->getAttackData(attackPatterns[i]->getAttacksCount() - 1)) + powerTiers[i]->getAttackPatternLoopDelay());
		} else {
			attackPatternTotalTimes.push_back(powerTiers[i]->getAttackPatternLoopDelay());
		}
		if (attackPatterns[i]->getAttacksCount() > 0) {
			focusedAttackPatternTotalTimes.push_back(std::get<0>(focusedAttackPatterns[i]->getAttackData(focusedAttackPatterns[i]->getAttacksCount() - 1)) + powerTiers[i]->getFocusedAttackPatternLoopDelay());
		} else {
			focusedAttackPatternTotalTimes.push_back(powerTiers[i]->getFocusedAttackPatternLoopDelay());
		}
	}

	currentPowerTierIndex = 0;
	registry.get<AnimatableSetComponent>(self).setAnimatableSet(powerTiers[currentPowerTierIndex]->getAnimatableSet());

	// Initialize time since last bomb activation such that user can attack/bomb at t=0
	timeSinceLastBombActivation = bombCooldowns[0];

	// Set current attack pattern
	switchToAttackPattern(attackPatterns[currentPowerTierIndex], attackPatternTotalTimes[0]);
}

bool PlayerTag::update(float deltaTime, const LevelPack & levelPack, EntityCreationQueue & queue, SpriteLoader & spriteLoader, entt::DefaultRegistry & registry, uint32_t self) {
	timeSinceLastBombActivation += deltaTime;

	if (isBombing) {
		// In the process of using a bomb

		while (currentBombAttackIndex + 1 < bombAttackPattern->getAttacksCount()) {
			auto nextAttack = bombAttackPattern->getAttackData(currentBombAttackIndex + 1);
			if (timeSinceLastBombActivation >= std::get<0>(nextAttack)) {
				currentBombAttackIndex++;
				levelPack.getGameplayAttack(std::get<1>(nextAttack), std::get<2>(nextAttack))->executeAsPlayer(queue, spriteLoader, registry, self, timeSinceLastBombActivation - std::get<0>(nextAttack), bombAttackPattern->getID());
			} else {
				break;
			}
		}
		// Do not loop the bomb attack pattern
		// Instead, switch back to the normal attack pattern
		if (currentBombAttackIndex + 1 == bombAttackPattern->getAttacksCount()) {
			if (focused) {
				switchToAttackPattern(focusedAttackPatterns[currentPowerTierIndex], focusedAttackPatternTotalTimes[currentPowerTierIndex]);
			} else {
				switchToAttackPattern(attackPatterns[currentPowerTierIndex], attackPatternTotalTimes[currentPowerTierIndex]);
			}
			isBombing = false;
		}
	}
	if (attacking) {
		timeSinceNewAttackPattern += deltaTime;

		while (currentAttackIndex + 1 < currentAttackPattern->getAttacksCount()) {
			auto nextAttack = currentAttackPattern->getAttackData(currentAttackIndex + 1);
			if (timeSinceNewAttackPattern >= std::get<0>(nextAttack)) {
				currentAttackIndex++;
				levelPack.getGameplayAttack(std::get<1>(nextAttack), std::get<2>(nextAttack))->executeAsPlayer(queue, spriteLoader, registry, self, timeSinceNewAttackPattern - std::get<0>(nextAttack), currentAttackPattern->getID());
			} else {
				break;
			}
		}
		// Loop the current attack pattern
		if (currentAttackIndex + 1 == currentAttackPattern->getAttacksCount()) {
			while (timeSinceNewAttackPattern >= currentAttackPatternTotalTime) {
				timeSinceNewAttackPattern -= currentAttackPatternTotalTime;
				currentAttackIndex = -1;
			}
		}
	}

	if (timeSinceLastBombActivation - deltaTime < bombCooldowns[currentPowerTierIndex] && timeSinceLastBombActivation >= bombCooldowns[currentPowerTierIndex] && deltaTime != 0) {
		return true;
	}
	return false;
}

void PlayerTag::activateBomb(entt::DefaultRegistry& registry, uint32_t self) {
	if (timeSinceLastBombActivation >= bombCooldowns[currentPowerTierIndex] && bombs > 0) {
		timeSinceLastBombActivation = 0;
		isBombing = true;
		currentBombAttackIndex = -1;
		bombAttackPattern = bombAttackPatterns[currentPowerTierIndex];

		bombs--;
		onBombCountChange();

		// Make player invincible for some time
		registry.get<HitboxComponent>(self).disable(bombInvincibilityTime);
		auto& sprite = registry.get<SpriteComponent>(self);
		sprite.setEffectAnimation(std::make_unique<FlashWhiteSEA>(sprite.getSprite(), bombInvincibilityTime));
	}
}

int PlayerTag::getPowerTierCount() {
	return powerTiers.size();
}

int PlayerTag::getPowerToNextTier() {
	return powerTiers[currentPowerTierIndex]->getPowerToNextTier();
}

void PlayerTag::setFocused(bool focused) {
	if (!this->focused && focused) {
		switchToAttackPattern(focusedAttackPatterns[currentPowerTierIndex], focusedAttackPatternTotalTimes[currentPowerTierIndex]);
	} else if (this->focused && !focused) {
		switchToAttackPattern(attackPatterns[currentPowerTierIndex], attackPatternTotalTimes[currentPowerTierIndex]);
	}
	this->focused = focused;
}

void PlayerTag::increasePower(entt::DefaultRegistry& registry, uint32_t self, int power, int pointsPerExtraPower) {
	if (currentPower + power >= powerTiers[currentPowerTierIndex]->getPowerToNextTier()) {
		if (currentPowerTierIndex + 1 < powerTiers.size()) {
			// Power tier up

			currentPowerTierIndex++;
			currentPower += power - powerTiers[currentPowerTierIndex]->getPowerToNextTier();
			// Change attack pattern
			if (focused) {
				switchToAttackPattern(focusedAttackPatterns[currentPowerTierIndex], focusedAttackPatternTotalTimes[currentPowerTierIndex]);
			} else {
				switchToAttackPattern(attackPatterns[currentPowerTierIndex], attackPatternTotalTimes[currentPowerTierIndex]);
			}

			// Update this entity's EntityAnimatableSet
			registry.get<AnimatableSetComponent>(self).setAnimatableSet(powerTiers[currentPowerTierIndex]->getAnimatableSet());
		} else {
			// Already reached power cap

			currentPower += power;
			// Increase points
			registry.get<LevelManagerTag>().addPoints(pointsPerExtraPower * (currentPower - powerTiers[currentPowerTierIndex]->getPowerToNextTier()));
			currentPower = powerTiers[currentPowerTierIndex]->getPowerToNextTier();
		}
	} else {
		currentPower += power;
	}
	onPowerChange();
}

void PlayerTag::gainBombs(entt::DefaultRegistry& registry, int amount, int pointsPerExtraBomb) {
	if (bombs + amount > maxBombs) {
		// Increase points depending on number of extra bombs
		registry.get<LevelManagerTag>().addPoints(pointsPerExtraBomb * (bombs + amount - maxBombs));

		bombs = maxBombs;
	} else {
		bombs += amount;
	}
	onBombCountChange();
}

void PlayerTag::switchToAttackPattern(std::shared_ptr<EditorAttackPattern> newAttackPattern, float newAttackPatternTotalTime) {
	timeSinceNewAttackPattern = 0;
	currentAttackIndex = -1;
	currentAttackPattern = newAttackPattern;
	currentAttackPatternTotalTime = newAttackPatternTotalTime;
}

void PlayerTag::onPowerChange() {
	if (powerChangeSignal) {
		powerChangeSignal->publish(currentPowerTierIndex, powerTiers.size(), currentPower);
	}
}

void PlayerTag::onBombCountChange() {
	if (bombCountChangeSignal) {
		bombCountChangeSignal->publish(bombs);
	}
}

std::shared_ptr<entt::SigH<void(int, int, int)>> PlayerTag::getPowerChangeSignal() {
	if (powerChangeSignal) {
		return powerChangeSignal;
	}
	powerChangeSignal = std::make_shared<entt::SigH<void(int, int, int)>>();
	return powerChangeSignal;
}

std::shared_ptr<entt::SigH<void(int)>> PlayerTag::getBombCountChangeSignal() {
	if (bombCountChangeSignal) {
		return bombCountChangeSignal;
	}
	bombCountChangeSignal = std::make_shared<entt::SigH<void(int)>>();
	return bombCountChangeSignal;
}

void CollectibleComponent::activate(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t self) {
	// Item is activated, so begin moving towards the player

	auto speed = std::make_shared<PiecewiseTFV>();
	// Speed starts off quickly at 450 and slows to 350 by t=2
	speed->insertSegment(std::make_pair(0, std::make_shared<DampenedEndTFV>(450.0f, 350.0f, 2.0f, 2)));
	// Speed then maintains a constant 350 forever
	speed->insertSegment(std::make_pair(2.0f, std::make_shared<ConstantTFV>(350.0f)));
	auto newPath = std::make_shared<HomingMP>(std::numeric_limits<float>::max(), speed, std::make_shared<ConstantTFV>(0.12f), self, registry.attachee<PlayerTag>(), registry);
	registry.get<MovementPathComponent>(self).setPath(queue, registry, self, registry.get<PositionComponent>(self), newPath, 0);
	// Make sure item can't despawn
	registry.get<DespawnComponent>(self).setMaxTime(std::numeric_limits<float>::max());
	activated = true;
}

std::shared_ptr<Item> CollectibleComponent::getItem() {
	return item;
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

void EnemyBulletComponent::update(float deltaTime) {
	for (auto& it = ignoredEntities.begin(); it != ignoredEntities.end(); it++) {
		it->second -= deltaTime;
	}
}

void EnemyBulletComponent::onCollision(uint32_t collidedWith) {
	if (onCollisionAction != PIERCE_ENTITY) return;

	if (ignoredEntities.find(collidedWith) == ignoredEntities.end()) {
		ignoredEntities.insert(std::make_pair(collidedWith, pierceResetTime));
	} else {
		ignoredEntities[collidedWith] = pierceResetTime;
	}
}

bool EnemyBulletComponent::isValidCollision(uint32_t collidingWith) {
	if (onCollisionAction != PIERCE_ENTITY) return true;
	return ignoredEntities.find(collidingWith) == ignoredEntities.end() || ignoredEntities[collidingWith] <= 0;
}

BULLET_ON_COLLISION_ACTION EnemyBulletComponent::getOnCollisionAction() {
	return onCollisionAction;
}

void PlayerBulletComponent::update(float deltaTime) {
	for (auto& it = ignoredEntities.begin(); it != ignoredEntities.end(); it++) {
		it->second -= deltaTime;
	}
}

void PlayerBulletComponent::onCollision(uint32_t collidedWith) {
	if (onCollisionAction != PIERCE_ENTITY) return;

	if (ignoredEntities.find(collidedWith) == ignoredEntities.end()) {
		ignoredEntities.insert(std::make_pair(collidedWith, pierceResetTime));
	} else {
		ignoredEntities[collidedWith] = pierceResetTime;
	}
}

bool PlayerBulletComponent::isValidCollision(uint32_t collidingWith) {
	if (onCollisionAction != PIERCE_ENTITY) return true;
	return ignoredEntities.find(collidingWith) == ignoredEntities.end() || ignoredEntities[collidingWith] <= 0;
}

BULLET_ON_COLLISION_ACTION PlayerBulletComponent::getOnCollisionAction() {
	return onCollisionAction;
}

LevelPack* LevelManagerTag::getLevelPack() {
	return levelPack;
}

std::shared_ptr<entt::SigH<void(int)>> LevelManagerTag::getPointsChangeSignal() {
	if (pointsChangeSignal) {
		return pointsChangeSignal;
	}
	pointsChangeSignal = std::make_shared<entt::SigH<void(int)>>();
	return pointsChangeSignal;
}

std::shared_ptr<entt::SigH<void(uint32_t)>> LevelManagerTag::getEnemySpawnSignal() {
	if (enemySpawnSignal) {
		return enemySpawnSignal;
	}
	enemySpawnSignal = std::make_shared<entt::SigH<void(uint32_t)>>();
	return enemySpawnSignal;
}

void LevelManagerTag::onEnemySpawn(uint32_t enemy) {
	timeSinceLastEnemySpawn = 0;
	if (enemySpawnSignal) {
		enemySpawnSignal->publish(enemy);
	}
}

void LevelManagerTag::onPointsChange() {
	if (pointsChangeSignal) {
		pointsChangeSignal->publish(points);
	}
}

bool HealthComponent::takeDamage(int damage) {
	health -= damage;
	onHealthChange();
	return health <= 0;
}

void HealthComponent::heal(int amount) {
	health += amount;
	if (health > maxHealth) {
		health = maxHealth;
	}
	onHealthChange();
}

std::shared_ptr<entt::SigH<void(int, int)>> HealthComponent::getHPChangeSignal() {
	if (onHealthChangeSignal) {
		return onHealthChangeSignal;
	}
	onHealthChangeSignal = std::make_shared<entt::SigH<void(int, int)>>();
	return onHealthChangeSignal;
}

void HealthComponent::onHealthChange() {
	if (onHealthChangeSignal) {
		onHealthChangeSignal->publish(health, maxHealth);
	}
}

std::shared_ptr<entt::SigH<void(uint32_t)>> DespawnComponent::getDespawnSignal() {
	if (despawnSignal) {
		return despawnSignal;
	}
	despawnSignal = std::make_shared<entt::SigH<void(uint32_t)>>();
	return despawnSignal;
}

void DespawnComponent::onDespawn(uint32_t self) {
	if (despawnSignal) {
		despawnSignal->publish(self);
	}
}
