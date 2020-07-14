#pragma once
#include <memory>
#include <vector>
#include <string>
#include <SFML/Graphics.hpp>
#include "TextMarshallable.h"
#include "EntityAnimatableSet.h"
#include "SpriteLoader.h"
#include "AudioPlayer.h"
#include "ExpressionCompilable.h"
#include "LevelPackObject.h"

class PlayerPowerTier : public LevelPackObject {
public:
	inline PlayerPowerTier() {}
	inline PlayerPowerTier(EntityAnimatableSet animatableSet, int attackPatternID, std::string attackPatternLoopDelay, int focusedAttackPatternID, 
		std::string focusedAttackPatternLoopDelay, int bombAttackPatternID, std::string bombCooldown, std::string powerToNextTier) :
		animatableSet(animatableSet), attackPatternID(attackPatternID), attackPatternLoopDelay(attackPatternLoopDelay), focusedAttackPatternID(focusedAttackPatternID), 
		focusedAttackPatternLoopDelay(focusedAttackPatternLoopDelay), bombAttackPatternID(bombAttackPatternID), bombCooldown(bombCooldown), powerToNextTier(powerToNextTier) {}

	std::shared_ptr<LevelPackObject> clone() const;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	inline const EntityAnimatableSet& getAnimatableSet() const { return animatableSet; }
	inline int getAttackPatternID() const { return attackPatternID; }
	inline float getAttackPatternLoopDelay() const { return attackPatternLoopDelayExprCompiledValue; }
	inline int getFocusedAttackPatternID() const { return focusedAttackPatternID; }
	inline float getFocusedAttackPatternLoopDelay() const { return focusedAttackPatternLoopDelayExprCompiledValue; }
	inline int getBombAttackPatternID() const { return bombAttackPatternID; }
	inline float getBombCooldown() const { return bombCooldownExprCompiledValue; }
	inline int getPowerToNextTier() const { return powerToNextTierExprCompiledValue; }
	inline ExprSymbolTable getAttackPatternSymbolsDefiner() { return attackPatternSymbolsDefiner; }
	inline exprtk::symbol_table<float> getCompiledAttackPatternSymbolsDefiner() { return compiledAttackPatternSymbolsDefiner; }

	inline void setAttackPatternID(int id) { attackPatternID = id; }
	inline void setAttackPatternLoopDelay(std::string attackPatternLoopDelay) { this->attackPatternLoopDelay = attackPatternLoopDelay; }
	inline void setFocusedAttackPatternID(int id) { focusedAttackPatternID = id; }
	inline void setFocusedAttackPatternLoopDelay(std::string focusedAttackPatternLoopDelay) { this->focusedAttackPatternLoopDelay = focusedAttackPatternLoopDelay; }
	inline void setBombAttackPatternID(int id) { bombAttackPatternID = id; }
	inline void setBombCooldown(std::string bombCooldown) { this->bombCooldown = bombCooldown; }
	inline void setAttackPatternSymbolsDefiner(ExprSymbolTable attackPatternSymbolsDefiner) { this->attackPatternSymbolsDefiner = attackPatternSymbolsDefiner; }

private:
	EntityAnimatableSet animatableSet;

	int attackPatternID;
	ExprSymbolTable attackPatternSymbolsDefiner;
	exprtk::symbol_table<float> compiledAttackPatternSymbolsDefiner;
	// Time after attack pattern ends until it starts looping again
	DEFINE_EXPRESSION_VARIABLE(attackPatternLoopDelay, float)

	int focusedAttackPatternID;
	DEFINE_EXPRESSION_VARIABLE(focusedAttackPatternLoopDelay, float)

	// Attack pattern ID of the attack pattern that plays when a bomb is used
	int bombAttackPatternID;
	// Time after a bomb is used that the player can use another bomb. Should be greater than the time to go through every attack in the bomb attack pattern.
	DEFINE_EXPRESSION_VARIABLE(bombCooldown, float)

	// Amount of power needed to reach the next tier
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(powerToNextTier, int, 20)
};

class EditorPlayer : public LevelPackObject {
public:
	inline EditorPlayer() {}

	std::shared_ptr<LevelPackObject> clone() const;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	inline int getInitialHealth() const { return initialHealthExprCompiledValue; }
	inline int getMaxHealth() const { return maxHealthExprCompiledValue; }
	inline float getSpeed() const { return speedExprCompiledValue; }
	inline float getFocusedSpeed() const { return focusedSpeedExprCompiledValue; }
	inline std::vector<std::shared_ptr<PlayerPowerTier>> getPowerTiers() const { return powerTiers; }
	inline float getHitboxRadius() const { return hitboxRadiusExprCompiledValue; }
	inline float getHitboxPosX() const { return hitboxPosXExprCompiledValue; }
	inline float getHitboxPosY() const { return hitboxPosYExprCompiledValue; }
	inline float getInvulnerabilityTime() const { return invulnerabilityTimeExprCompiledValue; }
	inline SoundSettings getHurtSound() const { return hurtSound; }
	inline SoundSettings getDeathSound() const { return deathSound; }
	inline bool getSmoothPlayerHPBar() const { return smoothPlayerHPBar; }
	inline sf::Color getPlayerHPBarColor() const { return playerHPBarColor; }
	inline Animatable getDiscretePlayerHPSprite() const { return discretePlayerHPSprite; }
	inline int getInitialBombs() const { return initialBombsExprCompiledValue; }
	inline int getMaxBombs() const { return maxBombsExprCompiledValue; }
	inline Animatable getBombSprite() const { return bombSprite; }
	inline SoundSettings getBombReadySound() const { return bombReadySound; }
	inline float getBombInvincibilityTime() const { return bombInvincibilityTimeExprCompiledValue; }
	inline bool usesAttackPattern(int attackPatternID) const { return attackPatternIDCount.count(attackPatternID) > 0 && attackPatternIDCount.at(attackPatternID) > 0; }

	inline std::shared_ptr<PlayerPowerTier> getPowerTier(int index) { return powerTiers[index]; }

	inline void setHurtSound(SoundSettings hurtSound) { this->hurtSound = hurtSound; }
	inline void setDeathSound(SoundSettings deathSound) { this->deathSound = deathSound; }
	inline void setBombReadySound(SoundSettings bombReadySound) { this->bombReadySound = bombReadySound; }
	inline void setInitialHealth(std::string initialHealth) { this->initialHealth = initialHealth; }
	inline void setMaxHealth(std::string maxHealth) { this->maxHealth = maxHealth; }
	inline void setSpeed(std::string speed) { this->speed = speed; }
	inline void setFocusedSpeed(std::string focusedSpeed) { this->focusedSpeed = focusedSpeed; }
	inline void setHitboxRadius(std::string hitboxRadius) { this->hitboxRadius = hitboxRadius; }
	inline void setHitboxPosX(std::string hitboxPosX) { this->hitboxPosX = hitboxPosX; }
	inline void setHitboxPosY(std::string hitboxPosY) { this->hitboxPosY = hitboxPosY; }
	inline void setInvulnerabilityTime(std::string invulnerabilityTime) { this->invulnerabilityTime = invulnerabilityTime; }
	inline void insertPowerTier(int index, std::shared_ptr<PlayerPowerTier> powerTier) { 
		powerTiers.insert(powerTiers.begin() + index, powerTier);

		// Update attackPatternIDCount
		int attackPatternID = powerTier->getAttackPatternID();
		if (attackPatternIDCount.count(attackPatternID) == 0) {
			attackPatternIDCount[attackPatternID] = 1;
		} else {
			attackPatternIDCount[attackPatternID]++;
		}
	}
	inline void removePowerTier(int index) {
		int attackPatternID = powerTiers[index]->getAttackPatternID();
		powerTiers.erase(powerTiers.begin() + index);
		attackPatternIDCount[attackPatternID]--;

		// Update attackPatternIDCount
		if (attackPatternID >= 0 && attackPatternIDCount.count(attackPatternID) > 0) {
			attackPatternIDCount.at(attackPatternID)--;
			if (attackPatternIDCount.at(attackPatternID) == 0) {
				attackPatternIDCount.erase(attackPatternID);
			}
		}
	}
	inline void setSmoothPlayerHPBar(bool smoothPlayerHPBar) { this->smoothPlayerHPBar = smoothPlayerHPBar; }
	inline void setPlayerHPBarColor(sf::Color playerHPBarColor) { this->playerHPBarColor = playerHPBarColor; }
	inline void setDiscretePlayerHPSprite(Animatable discretePlayerHPSprite) { this->discretePlayerHPSprite = discretePlayerHPSprite; }
	inline void setInitialBombs(std::string initialBombs) { this->initialBombs = initialBombs; }
	inline void setMaxBombs(std::string initialBombs) { this->initialBombs = maxBombs; }
	inline void setBombSprite(Animatable bombSprite) { this->bombSprite = bombSprite; }
	inline void setBombInvincibilityTime(float bombInvincibilityTime) {	this->bombInvincibilityTime = bombInvincibilityTime; }

private:
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(initialHealth, float, 3)
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(maxHealth, float, 5)
	// Default player speed
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(speed, float, 120)
	// Player speed when holding focus key
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(focusedSpeed, float, 40)

	// Radius of the player's hitbox
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(hitboxRadius, float, 1)
	// Local position of hitbox
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(hitboxPosX, float, 0)
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(hitboxPosY, float, 0)

	// Time player is invulnerable for when hit by an enemy bullet
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(invulnerabilityTime, float, 2)

	/*
	The player's power tier increase every POWER_PER_POWER_TIER power, which come from power packs dropped by enemies.
	*/
	std::vector<std::shared_ptr<PlayerPowerTier>> powerTiers;

	SoundSettings hurtSound;
	SoundSettings deathSound;

	// If this is true, the HP bar will be a progress bar. If false, there will be a discretePlayerHPSprite displayed for each health the player has. 
	bool smoothPlayerHPBar = false;
	sf::Color playerHPBarColor = sf::Color::Red;
	// Must be a sprite. Only used if smoothPlayerHPBar is false.
	Animatable discretePlayerHPSprite;

	// The sprite shown on the GUI to denote a bomb. Must be a sprite.
	Animatable bombSprite;

	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(initialBombs, int, 2)
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(maxBombs, int, 6)

	// Sound played when bomb is off cooldown
	SoundSettings bombReadySound;

	// Amount of time player is invincible for after activating a bomb
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(bombInvincibilityTime, float, 5)

	// Maps an EditorAttackPattern's ID to the number of times it appears in powerTiers.
	// This map isn't saved in format() but is reconstructed in load().
	std::map<int, int> attackPatternIDCount;
};