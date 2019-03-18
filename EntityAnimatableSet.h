#pragma once
#include <memory>
#include <string>
#include <utility>
#include "TextMarshallable.h"

class Animatable : public TextMarshallable {
public:
	inline Animatable() {}
	inline Animatable(std::string animatableName, std::string spriteSheetName, bool animatableIsSprite) : animatableName(animatableName), spriteSheetName(spriteSheetName), animatableIsSprite(animatableIsSprite) {}

	std::string format() override;
	void load(std::string formattedString) override;

	inline std::string getAnimatableName() { return animatableName; }
	inline std::string getSpriteSheetName() { return spriteSheetName; }
	inline bool isSprite() { return animatableIsSprite; }

private:
	// Name of sprite/animation
	std::string animatableName;
	// Name of sprite sheet the animatable is in
	std::string spriteSheetName;
	// True if the associated animatable is a sprite. False if it's an animation
	bool animatableIsSprite;
};

/*
The set of animatables associated with each enemy phase
or with each player power tier.
*/
class EntityAnimatableSet : public TextMarshallable {
public:
	inline EntityAnimatableSet() {}
	inline EntityAnimatableSet(Animatable idle, Animatable movement, Animatable attack, Animatable death) : idleAnimatable(idle), movementAnimatable(movement), attackAnimatable(attack), deathAnimatable(death) {}

	std::string format() override;
	void load(std::string formattedString) override;

	inline Animatable getIdleAnimatable() { return idleAnimatable; }
	inline Animatable getMovementAnimatable() { return movementAnimatable; }
	inline Animatable getAttackAnimatable() { return attackAnimatable; }
	inline Animatable getDeathAnimatable() { return deathAnimatable; }

private:
	// Animatable used when an entity is idle
	Animatable idleAnimatable;
	// Animatable used when an entity is moving
	Animatable movementAnimatable;
	// Animatable used when an entity attacks
	Animatable attackAnimatable;
	// Animatable used when an entity dies
	Animatable deathAnimatable;
};