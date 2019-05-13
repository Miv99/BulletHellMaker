#pragma once
#include <memory>
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include "TextMarshallable.h"
#include "EntityAnimatableSet.h"
#include "AudioPlayer.h"

class PlayerPowerTier : public TextMarshallable {
public:
	inline PlayerPowerTier() {}
	inline PlayerPowerTier(EntityAnimatableSet animatableSet, int attackPatternID, float attackPatternLoopDelay, int focusedAttackPatternID, float focusedAttackPatternLoopDelay, int bombAttackPatternID, float bombCooldown) :
		animatableSet(animatableSet), attackPatternID(attackPatternID), attackPatternLoopDelay(attackPatternLoopDelay), focusedAttackPatternID(focusedAttackPatternID), focusedAttackPatternLoopDelay(focusedAttackPatternLoopDelay), bombAttackPatternID(bombAttackPatternID), bombCooldown(bombCooldown) {}

	std::string format() override;
	void load(std::string formattedString) override;

	inline const EntityAnimatableSet& getAnimatableSet() const { return animatableSet; }
	inline int getAttackPatternID() const { return attackPatternID; }
	inline float getAttackPatternLoopDelay() const { return attackPatternLoopDelay; }
	inline int getFocusedAttackPatternID() const { return focusedAttackPatternID; }
	inline float getFocusedAttackPatternLoopDelay() const { return focusedAttackPatternLoopDelay; }
	inline int getBombAttackPatternID() const { return bombAttackPatternID; }
	inline float getBombCooldown() const { return bombCooldown; }

	inline void setAttackPatternID(int id) { attackPatternID = id; }
	inline void setAttackPatternLoopDelay(float attackPatternLoopDelay) { this->attackPatternLoopDelay = attackPatternLoopDelay; }
	inline void setFocusedAttackPatternID(int id) { focusedAttackPatternID = id; }
	inline void setFocusedAttackPatternLoopDelay(float focusedAttackPatternLoopDelay) { this->focusedAttackPatternLoopDelay = focusedAttackPatternLoopDelay; }
	inline void setBombAttackPatternID(int id) { bombAttackPatternID = id; }
	inline void setBombCooldown(float bombCooldown) { this->bombCooldown = bombCooldown; }

private:
	EntityAnimatableSet animatableSet;

	int attackPatternID;
	// Time after attack pattern ends until it starts looping again
	float attackPatternLoopDelay;
	int focusedAttackPatternID;
	float focusedAttackPatternLoopDelay;

	// Attack pattern ID of the attack pattern that plays when a bomb is used
	int bombAttackPatternID;
	// Time after a bomb is used that the player can use another bomb. Should be greater than the time to go through every attack in the bomb attack pattern.
	float bombCooldown;
};

class EditorPlayer : public TextMarshallable {
public:
	inline EditorPlayer() {}
	inline EditorPlayer(float initialHealth, float maxHealth, float speed, float focusedSpeed, float hitboxRadius, float hitboxPosX, float hitboxPosY, float invulnerabilityTime, 
		std::vector<PlayerPowerTier> powerTiers, SoundSettings hurtSound, SoundSettings deathSound, int initialBombs, int maxBombs, Animatable bombSprite, SoundSettings bombReadySound) :
		initialHealth(initialHealth), maxHealth(maxHealth), speed(speed), focusedSpeed(focusedSpeed), hitboxRadius(hitboxRadius), hitboxPosX(hitboxPosX), hitboxPosY(hitboxPosY), 
		invulnerabilityTime(invulnerabilityTime), powerTiers(powerTiers), hurtSound(hurtSound), deathSound(deathSound), smoothPlayerHPBar(true),
		initialBombs(initialBombs), maxBombs(maxBombs), bombSprite(bombSprite), bombReadySound(bombReadySound) {}
	inline EditorPlayer(float initialHealth, float maxHealth, float speed, float focusedSpeed, float hitboxRadius, float hitboxPosX, float hitboxPosY, float invulnerabilityTime, 
		std::vector<PlayerPowerTier> powerTiers, SoundSettings hurtSound, SoundSettings deathSound, Animatable discretePlayerHPSprite, int initialBombs, int maxBombs, Animatable bombSprite, SoundSettings bombReadySound) :
		initialHealth(initialHealth), maxHealth(maxHealth), speed(speed), focusedSpeed(focusedSpeed), hitboxRadius(hitboxRadius), hitboxPosX(hitboxPosX), hitboxPosY(hitboxPosY), 
		invulnerabilityTime(invulnerabilityTime), powerTiers(powerTiers), hurtSound(hurtSound), deathSound(deathSound), smoothPlayerHPBar(false), discretePlayerHPSprite(discretePlayerHPSprite),
		initialBombs(initialBombs), maxBombs(maxBombs), bombSprite(bombSprite), bombReadySound(bombReadySound) {}

	std::string format() override;
	void load(std::string formattedString) override;

	inline int getInitialHealth() { return initialHealth; }
	inline int getMaxHealth() { return maxHealth; }
	inline float getSpeed() { return speed; }
	inline float getFocusedSpeed() { return focusedSpeed; }
	inline const std::vector<PlayerPowerTier> getPowerTiers() { return powerTiers; }
	inline float getHitboxRadius() { return hitboxRadius; }
	inline float getHitboxPosX() { return hitboxPosX; }
	inline float getHitboxPosY() { return hitboxPosY; }
	inline float getInvulnerabilityTime() { return invulnerabilityTime; }
	// Returns a reference
	inline SoundSettings& getHurtSound() { return hurtSound; }
	// Returns a reference
	inline SoundSettings& getDeathSound() { return deathSound; }
	inline bool getSmoothPlayerHPBar() { return smoothPlayerHPBar; }
	inline sf::Color getPlayerHPBarColor() { return playerHPBarColor; }
	inline Animatable getDiscretePlayerHPSprite() { return discretePlayerHPSprite; }
	inline int getInitialBombs() { return initialBombs; }
	inline int getMaxBombs() { return maxBombs; }
	inline Animatable getBombSprite() { return bombSprite; }
	// Returns a reference
	inline SoundSettings& getBombReadySound() { return bombReadySound; }

	/*
	Returns a reference to the power tier.
	*/
	inline PlayerPowerTier& getPowerTier(int index) { return powerTiers[index]; }

	inline void setInitialHealth(int initialHealth) { this->initialHealth = initialHealth; }
	inline void setMaxHealth(int maxHealth) { this->maxHealth = maxHealth; }
	inline void setSpeed(float speed) { this->speed = speed; }
	inline void setFocusedSpeed(float focusedSpeed) { this->focusedSpeed = focusedSpeed; }
	inline void setHitboxRadius(float hitboxRadius) { this->hitboxRadius = hitboxRadius; }
	inline void setHitboxPosX(float hitboxPosX) { this->hitboxPosX = hitboxPosX; }
	inline void setHitboxPosY(float hitboxPosY) { this->hitboxPosY = hitboxPosY; }
	inline void setInvulnerabilityTime(float invulnerabilityTime) { this->invulnerabilityTime = invulnerabilityTime; }
	inline void insertPowerTier(int index, PlayerPowerTier powerTier) { powerTiers.insert(powerTiers.begin() + index, powerTier); }
	inline void removePowerTier(int index) { powerTiers.erase(powerTiers.begin() + index); }
	inline void setSmoothPlayerHPBar(bool smoothPlayerHPBar) { this->smoothPlayerHPBar = smoothPlayerHPBar; }
	inline void setPlayerHPBarColor(sf::Color playerHPBarColor) { this->playerHPBarColor = playerHPBarColor; }
	inline void setDiscretePlayerHPSprite(Animatable discretePlayerHPSprite) { this->discretePlayerHPSprite = discretePlayerHPSprite; }
	inline void getInitialBombs(int initialBombs) { this->initialBombs = initialBombs; }
	inline void getMaxBombs(int initialBombs) { this->initialBombs = maxBombs; }
	inline void setBombSprite(Animatable bombSprite) { this->bombSprite = bombSprite; }

private:
	int initialHealth = 3;
	int maxHealth = 5;
	// Default player speed
	float speed = 120;
	// Player speed when holding focus key
	float focusedSpeed = 40;

	// Radius of the hitbox associated with this enemy
	float hitboxRadius = 1;
	// Local position of hitbox
	float hitboxPosX = 0, hitboxPosY = 0;

	// Time player is invulnerable for when hit by an enemy bullet
	float invulnerabilityTime = 2.0f;
	
	/*
	The player's power tier increase every POWER_PER_POWER_TIER power, which come from power packs dropped by enemies.
	*/
	std::vector<PlayerPowerTier> powerTiers;

	SoundSettings hurtSound;
	SoundSettings deathSound;

	// If this is true, the HP bar will be a progress bar. If false, there will be a discretePlayerHPSprite displayed for each health the player has. 
	bool smoothPlayerHPBar = false;
	sf::Color playerHPBarColor = sf::Color::Red;
	// Must be a sprite. Only used if smoothPlayerHPBar is false.
	Animatable discretePlayerHPSprite;

	// The sprite shown on the GUI to denote a bomb. Must be a sprite.
	Animatable bombSprite;

	int initialBombs = 2;
	int maxBombs = 6;

	// Sound played when bomb is off cooldown
	SoundSettings bombReadySound;
};