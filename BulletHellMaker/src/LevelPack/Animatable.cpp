#include <LevelPack/Animatable.h>

std::string Animatable::format() const {
	return formatString(animatableName) + formatString(spriteSheetName) + formatBool(animatableIsSprite) + tos(static_cast<int>(rotationType));
}

void Animatable::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	animatableName = items[0];
	spriteSheetName = items[1];
	animatableIsSprite = unformatBool(items[2]);
	rotationType = static_cast<ROTATION_TYPE>(std::stoi(items[3]));
}