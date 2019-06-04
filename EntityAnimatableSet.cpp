#include "EntityAnimatableSet.h"
#include "DeathAction.h"

EntityAnimatableSet::EntityAnimatableSet(Animatable idle, Animatable movement, Animatable attack, std::shared_ptr<DeathAction> deathAction) : idleAnimatable(idle), movementAnimatable(movement), attackAnimatable(attack), deathAction(deathAction) {
}

std::string EntityAnimatableSet::format() {
	return "(" + idleAnimatable.format() + ")" + tm_delim + "(" + movementAnimatable.format() + ")" + tm_delim + "(" + attackAnimatable.format() + ")" + tm_delim + "(" + deathAction->format() + ")";
}

void EntityAnimatableSet::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	idleAnimatable.load(items[0]);
	movementAnimatable.load(items[1]);
	attackAnimatable.load(items[2]);
	deathAction = DeathActionFactory::create(items[3]);
}

std::shared_ptr<DeathAction> EntityAnimatableSet::getDeathAction() {
	return deathAction;
}
