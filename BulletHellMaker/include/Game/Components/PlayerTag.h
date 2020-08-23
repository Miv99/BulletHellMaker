#pragma once
#include <entt/entt.hpp>

#include <Game/AudioPlayer.h>

class LevelPack;
class PlayerPowerTier;
class EntityCreationQueue;
class SpriteLoader;
class EditorAttackPattern;
class SoundSettings;

class PlayerTag {
public:
	PlayerTag(entt::DefaultRegistry& registry, const LevelPack& levelPack, uint32_t self, float speed, float focusedSpeed, float invulnerabilityTime, std::vector<std::shared_ptr<PlayerPowerTier>> powerTiers,
		SoundSettings hurtSound, SoundSettings deathSound, int initialBombs, int maxBombs, float bombInvincibilityTime);

	/*
	Returns true if bomb went off cooldown on this update call.
	*/
	bool update(float deltaTime, const LevelPack& levelPack, EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry& registry, uint32_t self);

	void activateBomb(entt::DefaultRegistry& registry, uint32_t self);

	float getSpeed() const;
	float getFocusedSpeed() const;
	float getInvulnerabilityTime() const;
	const SoundSettings& getHurtSound() const;
	const SoundSettings& getDeathSound() const;
	int getCurrentPowerTierIndex() const;
	int getPowerTierCount() const;
	int getCurrentPower() const;
	float getTimeSinceLastBombActivation() const;
	bool getFocused() const;
	int getBombCount() const;
	float getBombCooldown() const;
	int getPowerToNextTier() const;
	bool isDead() const;
	bool isInvincible() const;

	void setFocused(bool focused);
	void setAttacking(bool attacking);
	void setIsDead(bool isDead);
	void setIsInvincible(bool isInvincible);

	/*
	power - power to increase by
	pointsPerExtraPower - points the player should receive for every 1 power past the max
	*/
	void increasePower(entt::DefaultRegistry& registry, uint32_t self, int power, int pointsPerExtraPower);
	/*
	amount - amount of bombs to increase by
	pointsPerExtraBomb - points the player should receive for every 1 bomb past the max
	*/
	void gainBombs(entt::DefaultRegistry& registry, int amount, int pointsPerExtraBomb);

	void switchToAttackPattern(std::shared_ptr<EditorAttackPattern> newAttackPattern, float newAttackPatternTotalTime);

	/*
	Returns the signal that is emitted whenever the amount of power changes.
	Parameters: new power tier index, max number of power tiers, and new power value
	*/
	std::shared_ptr<entt::SigH<void(int, int, int)>> getPowerChangeSignal();
	/*
	Returns the signal that is emitted whenever the amount of bombs changes.
	Parameter: new number of bombs
	*/
	std::shared_ptr<entt::SigH<void(int)>> getBombCountChangeSignal();

private:
	bool dead = false;
	bool invincibile = false;

	float speed;
	float focusedSpeed;

	int bombs;
	int maxBombs;

	// Amount of time the player becomes invinicible for when they activate a bomb
	float bombInvincibilityTime;

	std::vector<std::shared_ptr<PlayerPowerTier>> powerTiers;
	int currentPowerTierIndex = 0;

	// Total time for every attack to execute in addition to the loop delay
	// Index corresponds to powerTiers indexing
	std::vector<float> attackPatternTotalTimes;
	std::vector<float> focusedAttackPatternTotalTimes;

	// Index corresponds to powerTiers indexing
	std::vector<std::shared_ptr<EditorAttackPattern>> attackPatterns;
	std::vector<std::shared_ptr<EditorAttackPattern>> focusedAttackPatterns;
	std::vector<std::shared_ptr<EditorAttackPattern>> bombAttackPatterns;
	std::vector<float> bombCooldowns;

	std::shared_ptr<EditorAttackPattern> currentAttackPattern;
	float currentAttackPatternTotalTime;
	float timeSinceNewAttackPattern = 0;
	int currentAttackIndex = -1;

	// If player is currently focused (user is holding focus button)
	bool focused = false;
	// If player is currently attacking
	bool attacking = false;

	int currentPower = 0;

	// Time player is invulnerable for when hit by an enemy bullet
	float invulnerabilityTime;

	SoundSettings hurtSound;
	SoundSettings deathSound;

	bool isBombing = false;
	int currentBombAttackIndex = -1;
	float timeSinceLastBombActivation;
	// The attack pattern of the current bomb
	std::shared_ptr<EditorAttackPattern> bombAttackPattern;

	// Emitted when currentPower changes.
	// Parameters: currentPowerTierIndex, number of power tiers, and currentPower
	std::shared_ptr<entt::SigH<void(int, int, int)>> powerChangeSignal;
	// Emitted when number of bombs changes.
	// Parameter: bomb count
	std::shared_ptr<entt::SigH<void(int)>> bombCountChangeSignal;

	/*
	Called whenever anything related to power or power tiers changes.
	*/
	void onPowerChange();
	/*
	Called whenever number of bombs changes
	*/
	void onBombCountChange();
};