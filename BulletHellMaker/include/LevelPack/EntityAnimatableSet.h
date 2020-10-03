#pragma once
#include <memory>
#include <string>
#include <utility>

#include <LevelPack/TextMarshallable.h>
#include <LevelPack/Animatable.h>

class DeathAction;

/*
The set of animatables associated with each enemy phase
or with each player power tier.
*/
class EntityAnimatableSet : public TextMarshallable {
public:
	EntityAnimatableSet();
	EntityAnimatableSet(Animatable idle, Animatable movement, Animatable attack, std::shared_ptr<DeathAction> deathAction);
	EntityAnimatableSet(Animatable idle, Animatable movement, Animatable attack);

	std::string format() const override;
	void load(std::string formattedString) override;

	inline Animatable getIdleAnimatable() const { return idleAnimatable; }
	inline Animatable getMovementAnimatable() const { return movementAnimatable; }
	inline Animatable getAttackAnimatable() const { return attackAnimatable; }
	std::shared_ptr<DeathAction> getDeathAction() const;

private:
	// Animatable used when an entity is idle; always loops
	Animatable idleAnimatable;
	// Animatable used when an entity is moving; always loops
	Animatable movementAnimatable;
	// Animatable used when an entity attacks; never loops
	Animatable attackAnimatable;
	// Action performed on death (includes the animatable to be displayed); its animatable never loops
	std::shared_ptr<DeathAction> deathAction;
};