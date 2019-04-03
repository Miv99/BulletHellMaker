#pragma once
#include <memory>
#include <string>
#include "AttackPattern.h"
#include "TextMarshallable.h"
#include "EntityAnimatableSet.h"
#include "Components.h"

class EditorPlayer : public TextMarshallable {
public:
	inline EditorPlayer() {}
	inline EditorPlayer(float initialHealth, float maxHealth, float speed, float focusedSpeed, EntityAnimatableSet animatableSet, float hitboxRadius, float hitboxPosX, float hitboxPosY, 
		std::shared_ptr<EditorAttackPattern> attackPattern, float attackPatternLoopDelay, std::shared_ptr<EditorAttackPattern> focusedAttackPattern, float focusedAttackPatternLoopDelay) :
		initialHealth(initialHealth), maxHealth(maxHealth), speed(speed), focusedSpeed(focusedSpeed), animatableSet(animatableSet), hitboxRadius(hitboxRadius), hitboxPosX(hitboxPosX), hitboxPosY(hitboxPosY),
		attackPattern(attackPattern), attackPatternLoopDelay(attackPatternLoopDelay), focusedAttackPattern(focusedAttackPattern), focusedAttackPatternLoopDelay(focusedAttackPatternLoopDelay) {}

	std::string format() override;
	void load(std::string formattedString) override;

	inline float getInitialHealth() { return initialHealth; }
	inline float getMaxHealth() { return maxHealth; }
	inline float getSpeed() { return speed; }
	inline float getFocusedSpeed() { return focusedSpeed; }
	inline EntityAnimatableSet getAnimatableSet() { return animatableSet; }
	inline float getHitboxRadius() { return hitboxRadius; }
	inline float getHitboxPosX() { return hitboxPosX; }
	inline float getHitboxPosY() { return hitboxPosY; }
	inline std::shared_ptr<EditorAttackPattern> getAttackPattern() { return attackPattern; }
	inline std::shared_ptr<EditorAttackPattern> getFocusedAttackPattern() { return focusedAttackPattern; }
	inline float getAttackPatternLoopDelay() { return attackPatternLoopDelay; }
	inline float getFocusedAttackPatternLoopDelay() { return focusedAttackPatternLoopDelay; }
	inline ROTATION_TYPE getRotationType() { return rotationType; }

	inline void setRotationType(ROTATION_TYPE rotationType) { this->rotationType = rotationType; }
	inline void setInitialHealth(int initialHealth) { this->initialHealth = initialHealth; }
	inline void setMaxHealth(int maxHealth) { this->maxHealth = maxHealth; }
	inline void setSpeed(float speed) { this->speed = speed; }
	inline void setFocusedSpeed(float focusedSpeed) { this->focusedSpeed = focusedSpeed; }
	inline void setAnimatableSet(EntityAnimatableSet animatableSet) { this->animatableSet = animatableSet; }
	inline void setHitboxRadius(float hitboxRadius) { this->hitboxRadius = hitboxRadius; }
	inline void setHitboxPosX(float hitboxPosX) { this->hitboxPosX = hitboxPosX; }
	inline void setHitboxPosY(float hitboxPosY) { this->hitboxPosY = hitboxPosY; }
	inline void setAttackPattern(std::shared_ptr<EditorAttackPattern> attackPattern) { this->attackPattern = attackPattern; }
	inline void setFocusedAttackPattern(std::shared_ptr<EditorAttackPattern> focusedAttackPattern) { this->focusedAttackPattern = focusedAttackPattern; }
	inline float setAttackPatternLoopDelay(float attackPatternLoopDelay) { this->attackPatternLoopDelay = attackPatternLoopDelay; }
	inline float setFocusedAttackPatternLoopDelay(float focusedAttackPatternLoopDelay) { this->focusedAttackPatternLoopDelay = focusedAttackPatternLoopDelay; }

private:
	int initialHealth;
	int maxHealth;
	// Default player speed
	float speed;
	// Player speed when holding focus key
	float focusedSpeed;
	EntityAnimatableSet animatableSet;

	// Radius of the hitbox associated with this enemy
	float hitboxRadius;
	// Local position of hitbox
	float hitboxPosX, hitboxPosY;

	std::shared_ptr<EditorAttackPattern> attackPattern;
	// Time after attack pattern ends until it starts looping again
	float attackPatternLoopDelay;
	std::shared_ptr<EditorAttackPattern> focusedAttackPattern;
	float focusedAttackPatternLoopDelay;

	ROTATION_TYPE rotationType;
};