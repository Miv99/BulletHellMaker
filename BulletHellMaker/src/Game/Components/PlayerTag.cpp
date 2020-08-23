#include <Game/Components/PlayerTag.h>

#include <LevelPack/LevelPack.h>
#include <LevelPack/Player.h>
#include <LevelPack/Attack.h>
#include <LevelPack/AttackPattern.h>
#include <Game/Components/SpriteComponent.h>
#include <Game/Components/HitboxComponent.h>
#include <Game/EntityCreationQueue.h>
#include <DataStructs/SpriteLoader.h>

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

bool PlayerTag::update(float deltaTime, const LevelPack& levelPack, EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry& registry, uint32_t self) {
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

float PlayerTag::getSpeed() const { 
	return speed;
}
float PlayerTag::getFocusedSpeed() const {
	return focusedSpeed;
}

float PlayerTag::getInvulnerabilityTime() const {
	return invulnerabilityTime;
}

const SoundSettings& PlayerTag::getHurtSound() const {
	return hurtSound;
}

const SoundSettings& PlayerTag::getDeathSound() const {
	return deathSound;
}
int PlayerTag::getCurrentPowerTierIndex() const {
	return currentPowerTierIndex;
}

int PlayerTag::getCurrentPower() const {
	return currentPower;
}

float PlayerTag::getTimeSinceLastBombActivation() const {
	return timeSinceLastBombActivation;
}

bool PlayerTag::getFocused() const  {
	return focused;
}

int PlayerTag::getBombCount() const {
	return bombs;
}

float PlayerTag::getBombCooldown() const {
	return bombCooldowns[currentPowerTierIndex];
}

bool PlayerTag::isDead() const {
	return dead;
}

bool PlayerTag::isInvincible() const {
	return invincibile;
}

void PlayerTag::setAttacking(bool attacking) { 
	this->attacking = attacking;
}

void PlayerTag::setIsDead(bool isDead) { 
	dead = isDead;
}

void PlayerTag::setIsInvincible(bool isInvincible) { 
	invincibile = isInvincible;
}

int PlayerTag::getPowerTierCount() const {
	return powerTiers.size();
}

int PlayerTag::getPowerToNextTier() const {
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

			// Update this component's entity's EntityAnimatableSet
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