#include "EditorMovablePointSpawnType.h"

std::string SpecificGlobalEMPSpawn::format() const {
	return formatString("SpecificGlobalEMPSpawn") + formatString(x) + formatString(y) + formatString(time);
}

void SpecificGlobalEMPSpawn::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	x = items[1];
	y = items[2];
	time = items[3];
}

MPSpawnInformation SpecificGlobalEMPSpawn::getSpawnInfo(entt::DefaultRegistry & registry, uint32_t entity, float timeLag) {
	return MPSpawnInformation{ false, NULL, sf::Vector2f(xExprCompiledValue, yExprCompiledValue) };
}

MPSpawnInformation SpecificGlobalEMPSpawn::getForcedDetachmentSpawnInfo(entt::DefaultRegistry & registry, float timeLag) {
	return MPSpawnInformation{ false, NULL, sf::Vector2f(xExprCompiledValue, yExprCompiledValue) };
}

std::string EntityRelativeEMPSpawn::format() const {
	return formatString("EntityRelativeEMPSpawn") + formatString(x) + formatString(y) + formatString(time);
}

void EntityRelativeEMPSpawn::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	x = items[1];
	y = items[2];
	time = items[3];
}

MPSpawnInformation EntityRelativeEMPSpawn::getSpawnInfo(entt::DefaultRegistry & registry, uint32_t entity, float timeLag) {
	// Assume that if the entity spawning this has no MovementPathComponent, it has stayed at the same global position for its entire lifespan
	if (registry.has<MovementPathComponent>(entity)) {
		auto& pos = registry.get<MovementPathComponent>(entity).getPreviousPosition(registry, timeLag);
		// Offset by the hitbox origin, if the entity has one
		if (registry.has<HitboxComponent>(entity)) {
			auto hitbox = registry.get<HitboxComponent>(entity);
			pos.x += hitbox.getX();
			pos.y += hitbox.getY();
		}
		return MPSpawnInformation{ false, NULL, sf::Vector2f(pos.x + xExprCompiledValue, pos.y + yExprCompiledValue) };
	} else {
		auto& pos = registry.get<PositionComponent>(entity);
		return MPSpawnInformation{ false, NULL, sf::Vector2f(pos.getX() + xExprCompiledValue, pos.getY() + yExprCompiledValue) };
	}
}

MPSpawnInformation EntityRelativeEMPSpawn::getForcedDetachmentSpawnInfo(entt::DefaultRegistry & registry, float timeLag) {
	return MPSpawnInformation{ false, NULL, sf::Vector2f(xExprCompiledValue, yExprCompiledValue) };
}

std::string EntityAttachedEMPSpawn::format() const {
	return formatString("EntityAttachedEMPSpawn") + formatString(x) + formatString(y) + formatString(time);
}

void EntityAttachedEMPSpawn::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	x = items[1];
	y = items[2];
	time = items[3];
}

MPSpawnInformation EntityAttachedEMPSpawn::getSpawnInfo(entt::DefaultRegistry & registry, uint32_t entity, float timeLag) {
	float offsetX = 0, offsetY = 0;
	// Offset by the hitbox origin, if the entity has one
	if (registry.has<HitboxComponent>(entity)) {
		auto hitbox = registry.get<HitboxComponent>(entity);
		offsetX += hitbox.getX();
		offsetY += hitbox.getY();
	}
	return MPSpawnInformation{ true, entity, sf::Vector2f(xExprCompiledValue, yExprCompiledValue) };
}

MPSpawnInformation EntityAttachedEMPSpawn::getForcedDetachmentSpawnInfo(entt::DefaultRegistry & registry, float timeLag) {
	return MPSpawnInformation{ false, NULL, sf::Vector2f(xExprCompiledValue, yExprCompiledValue) };
}

std::shared_ptr<EMPSpawnType> EMPSpawnTypeFactory::create(std::string formattedString) {
	auto name = split(formattedString, DELIMITER)[0];
	std::shared_ptr<EMPSpawnType> ptr;
	if (name == "SpecificGlobalEMPSpawn") {
		ptr = std::make_shared<SpecificGlobalEMPSpawn>();
	}
	else if (name == "EntityRelativeEMPSpawn") {
		ptr = std::make_shared<EntityRelativeEMPSpawn>();
	} 
	else if (name == "EntityAttachedEMPSpawn") {
		ptr = std::make_shared<EntityAttachedEMPSpawn>();
	}
	ptr->load(formattedString);
	return ptr;
}

bool EMPSpawnType::operator==(const EMPSpawnType& other) const {
	return time == other.time && x == other.x && y == other.y;
}
