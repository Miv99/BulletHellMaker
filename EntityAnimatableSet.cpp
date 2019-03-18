#include "EntityAnimatableSet.h"

std::string Animatable::format() {
	std::string res = "(" + animatableName + ")" + delim + "(" + spriteSheetName + ")" + delim;
	if (animatableIsSprite) {
		res += "1";
	} else {
		res += "0";
	}
	return res;
}

void Animatable::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	animatableName = items[0];
	spriteSheetName = items[1];
	if (std::stoi(items[2])) {
		animatableIsSprite = true;
	} else {
		animatableIsSprite = false;
	}
}

std::string EntityAnimatableSet::format() {
	return "(" + idleAnimatable.format() + ")" + delim + "(" + movementAnimatable.format() + ")" + delim + "(" + attackAnimatable.format() + ")" + delim + "(" + deathAnimatable.format() + ")";
}

void EntityAnimatableSet::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	idleAnimatable.load(items[0]);
	movementAnimatable.load(items[1]);
	attackAnimatable.load(items[2]);
	deathAnimatable.load(items[3]);
}
