#include "EditorMovablePointSpawnType.h"

std::string SpecificGlobalEMPSpawn::format() {
	std::string res = "";
	res += "SpecificGlobalEMPSpawn" + tm_delim;
	res += tos(x) + tm_delim;
	res += tos(y) + tm_delim;
	res += tos(time);
	return res;
}

void SpecificGlobalEMPSpawn::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	x = std::stof(items[1]);
	y = std::stof(items[2]);
	time = std::stof(items[3]);
}

MPSpawnInformation SpecificGlobalEMPSpawn::getSpawnInfo(entt::DefaultRegistry & registry, uint32_t entity, float timeLag) {
	return MPSpawnInformation{ false, NULL, sf::Vector2f(x, y) };
}

std::string EntityRelativeEMPSpawn::format() {
	std::string res = "";
	res += "EntityRelativeEMPSpawn" + tm_delim;
	res += tos(x) + tm_delim;
	res += tos(y) + tm_delim;
	res += tos(time);
	return res;
}

void EntityRelativeEMPSpawn::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	x = std::stof(items[1]);
	y = std::stof(items[2]);
	time = std::stof(items[3]);
}

MPSpawnInformation EntityRelativeEMPSpawn::getSpawnInfo(entt::DefaultRegistry & registry, uint32_t entity, float timeLag) {
	// Assume that if the entity spawning this has no MovementPathComponent, it has stayed at the same global position for its entire lifespan
	if (registry.has<MovementPathComponent>(entity)) {
		auto& pos = registry.get<MovementPathComponent>(entity).getPreviousPosition(registry, timeLag);
		return MPSpawnInformation{ false, NULL, sf::Vector2f(pos.x + x, pos.y + y) };
	} else {
		auto& pos = registry.get<PositionComponent>(entity);
		return MPSpawnInformation{ false, NULL, sf::Vector2f(pos.getX() + x, pos.getY() + y) };
	}
}

std::string EntityAttachedEMPSpawn::format() {
	std::string res = "";
	res += "EntityAttachedEMPSpawn" + tm_delim;
	res += tos(x) + tm_delim;
	res += tos(y) + tm_delim;
	res += tos(time);
	return res;
}

void EntityAttachedEMPSpawn::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	x = std::stof(items[1]);
	y = std::stof(items[2]);
	time = std::stof(items[3]);
}

MPSpawnInformation EntityAttachedEMPSpawn::getSpawnInfo(entt::DefaultRegistry & registry, uint32_t entity, float timeLag) {
	return MPSpawnInformation{ true, entity, sf::Vector2f(x, y) };
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
