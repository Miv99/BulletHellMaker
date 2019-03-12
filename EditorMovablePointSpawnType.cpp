#include "EditorMovablePointSpawnType.h"

std::string SpecificGlobalEMPSpawn::format() {
	std::string res = "";
	res += "SpecificGlobalEMPSpawn" + delim;
	res += tos(x) + delim;
	res += tos(y) + delim;
	res += tos(time);
	return res;
}

void SpecificGlobalEMPSpawn::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	x = std::stof(items[1]);
	y = std::stof(items[2]);
	time = std::stof(items[3]);
}

MPSpawnInformation SpecificGlobalEMPSpawn::getSpawnInfo(const entt::DefaultRegistry & registry, uint32_t entity) {
	return MPSpawnInformation{ false, NULL, sf::Vector2f(x, y) };
}

std::string EnemyRelativeEMPSpawn::format() {
	std::string res = "";
	res += "EnemyRelativeEMPSpawn" + delim;
	res += tos(x) + delim;
	res += tos(y) + delim;
	res += tos(time);
	return res;
}

void EnemyRelativeEMPSpawn::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	x = std::stof(items[1]);
	y = std::stof(items[2]);
	time = std::stof(items[3]);
}

MPSpawnInformation EnemyRelativeEMPSpawn::getSpawnInfo(const entt::DefaultRegistry & registry, uint32_t entity) {
	auto& posComponent = registry.get<PositionComponent>(entity);
	return MPSpawnInformation{ false, NULL, sf::Vector2f(posComponent.getX() + x, posComponent.getY() + y) };
}

std::string EnemyAttachedEMPSpawn::format() {
	std::string res = "";
	res += "EnemyAttachedEMPSpawn" + delim;
	res += tos(x) + delim;
	res += tos(y) + delim;
	res += tos(time);
	return res;
}

void EnemyAttachedEMPSpawn::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	x = std::stof(items[1]);
	y = std::stof(items[2]);
	time = std::stof(items[3]);
}

MPSpawnInformation EnemyAttachedEMPSpawn::getSpawnInfo(const entt::DefaultRegistry & registry, uint32_t entity) {
	return MPSpawnInformation{ true, entity, sf::Vector2f(x, y) };
}

std::shared_ptr<EMPSpawnType> EMPSpawnTypeFactory::create(std::string formattedString) {
	auto name = split(formattedString, DELIMITER)[0];
	std::shared_ptr<EMPSpawnType> ptr;
	if (name == "SpecificGlobalEMPSpawn") {
		ptr = std::make_shared<SpecificGlobalEMPSpawn>();
	}
	else if (name == "EnemyRelativeEMPSpawn") {
		ptr = std::make_shared<EnemyRelativeEMPSpawn>();
	} 
	else if (name == "EnemyAttachedEMPSpawn") {
		ptr = std::make_shared<EnemyAttachedEMPSpawn>();
	}
	ptr->load(formattedString);
	return ptr;
}
