#include <LevelPack/EntityAnimatableSet.h>

#include <LevelPack/DeathAction.h>

EntityAnimatableSet::EntityAnimatableSet() {
}

EntityAnimatableSet::EntityAnimatableSet(Animatable idle, Animatable movement, Animatable attack, std::shared_ptr<DeathAction> deathAction) : idleAnimatable(idle), movementAnimatable(movement), attackAnimatable(attack), deathAction(deathAction) {
}

EntityAnimatableSet::EntityAnimatableSet(Animatable idle, Animatable movement, Animatable attack) : idleAnimatable(idle), movementAnimatable(movement), attackAnimatable(attack), deathAction(std::make_shared<NullDeathAction>()) {
}

std::string EntityAnimatableSet::format() const {
	return formatTMObject(idleAnimatable) + formatTMObject(movementAnimatable) + formatTMObject(attackAnimatable) + formatTMObject(*deathAction);
}

void EntityAnimatableSet::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	idleAnimatable.load(items[0]);
	movementAnimatable.load(items[1]);
	attackAnimatable.load(items[2]);
	deathAction = DeathActionFactory::create(items[3]);
}

std::shared_ptr<DeathAction> EntityAnimatableSet::getDeathAction() {
	return deathAction;
}
