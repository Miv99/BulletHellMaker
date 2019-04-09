#include "Animatable.h"

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