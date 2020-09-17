#include <LevelPack/Animatable.h>

Animatable::Animatable() {
}

Animatable::Animatable(std::string animatableName, std::string spriteSheetName, bool animatableIsSprite, ROTATION_TYPE rotationType) 
	: animatableName(animatableName), spriteSheetName(spriteSheetName), animatableIsSprite(animatableIsSprite), rotationType(rotationType) {
}

std::string Animatable::format() const {
	return formatString(animatableName) + formatString(spriteSheetName) + formatBool(animatableIsSprite) + tos(static_cast<int>(rotationType));
}

void Animatable::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	animatableName = items.at(0);
	spriteSheetName = items.at(1);
	animatableIsSprite = unformatBool(items.at(2));
	rotationType = static_cast<ROTATION_TYPE>(std::stoi(items.at(3)));
}

bool Animatable::operator==(const Animatable& other) const {
	return animatableName == other.animatableName && spriteSheetName == other.spriteSheetName
		&& animatableIsSprite == other.animatableIsSprite && rotationType == other.rotationType;
}
