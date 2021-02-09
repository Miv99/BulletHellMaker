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
	idleAnimatable.load(items.at(0));
	movementAnimatable.load(items.at(1));
	attackAnimatable.load(items.at(2));
	deathAction = DeathActionFactory::create(items.at(3));
}

nlohmann::json EntityAnimatableSet::toJson() {
	return {
		{"idleAnimatable", idleAnimatable.toJson()},
		{"movementAnimatable", movementAnimatable.toJson()},
		{"attackAnimatable", attackAnimatable.toJson()},
		{"deathAction", deathAction->toJson()}
	};
}

void EntityAnimatableSet::load(const nlohmann::json& j) {
	if (j.contains("idleAnimatable")) {
		idleAnimatable.load(j.at("idleAnimatable"));
	} else {
		idleAnimatable = Animatable();
	}

	if (j.contains("movementAnimatable")) {
		movementAnimatable.load(j.at("movementAnimatable"));
	} else {
		movementAnimatable = Animatable();
	}

	if (j.contains("attackAnimatable")) {
		attackAnimatable.load(j.at("attackAnimatable"));
	} else {
		attackAnimatable = Animatable();
	}

	if (j.contains("deathAction")) {
		deathAction = DeathActionFactory::create(j.at("deathAction"));
	} else {
		deathAction = std::make_shared<NullDeathAction>();
	}
}

std::shared_ptr<DeathAction> EntityAnimatableSet::getDeathAction() const {
	return deathAction;
}
