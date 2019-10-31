#include "Animatable.h"

std::string Animatable::format() const {
	std::string res = "(" + animatableName + ")" + tm_delim + "(" + spriteSheetName + ")" + tm_delim;
	if (animatableIsSprite) {
		res += "1" + tm_delim;
	} else {
		res += "0" + tm_delim;
	}
	res += tos(static_cast<int>(rotationType));
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
	rotationType = static_cast<ROTATION_TYPE>(std::stoi(items[3]));
}