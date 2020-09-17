#include <LevelPack/EditorMovablePointSpawnType.h>

#include <Game/Components/HitboxComponent.h>
#include <Game/Components/MovementPathComponent.h>

EMPSpawnType::EMPSpawnType() {
}

EMPSpawnType::EMPSpawnType(std::string time, std::string x, std::string y) 
	: time(time), x(x), y(y) {
}

EMPSpawnType::EMPSpawnType(float time, float x, float y) 
	: timeExprCompiledValue(time), xExprCompiledValue(x), yExprCompiledValue(y) {
}

SpecificGlobalEMPSpawn::SpecificGlobalEMPSpawn() {
}

SpecificGlobalEMPSpawn::SpecificGlobalEMPSpawn(std::string time, std::string x, std::string y)
	: EMPSpawnType(time, x, y) {
}

SpecificGlobalEMPSpawn::SpecificGlobalEMPSpawn(float time, float x, float y) 
	: EMPSpawnType(time, x, y) {
}

std::shared_ptr<LevelPackObject> SpecificGlobalEMPSpawn::clone() const {
	auto clone = std::make_shared<SpecificGlobalEMPSpawn>();
	clone->load(format());
	return clone;
}

std::string SpecificGlobalEMPSpawn::format() const {
	return formatString("SpecificGlobalEMPSpawn") + formatString(x) + formatString(y) + formatString(time);
}

void SpecificGlobalEMPSpawn::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
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

EntityRelativeEMPSpawn::EntityRelativeEMPSpawn() {
}

EntityRelativeEMPSpawn::EntityRelativeEMPSpawn(std::string time, std::string x, std::string y)
	: EMPSpawnType(time, x, y) {
}

EntityRelativeEMPSpawn::EntityRelativeEMPSpawn(float time, float x, float y)
	: EMPSpawnType(time, x, y) {
}

std::shared_ptr<LevelPackObject> EntityRelativeEMPSpawn::clone() const {
	auto clone = std::make_shared<EntityRelativeEMPSpawn>();
	clone->load(format());
	return clone;
}

std::string EntityRelativeEMPSpawn::format() const {
	return formatString("EntityRelativeEMPSpawn") + formatString(x) + formatString(y) + formatString(time);
}

void EntityRelativeEMPSpawn::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
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

EntityAttachedEMPSpawn::EntityAttachedEMPSpawn() {
}

EntityAttachedEMPSpawn::EntityAttachedEMPSpawn(std::string time, std::string x, std::string y)
	: EMPSpawnType(time, x, y) {
}

EntityAttachedEMPSpawn::EntityAttachedEMPSpawn(float time, float x, float y)
	: EMPSpawnType(time, x, y) {
}

std::shared_ptr<LevelPackObject> EntityAttachedEMPSpawn::clone() const {
	auto clone = std::make_shared<EntityAttachedEMPSpawn>();
	clone->load(format());
	return clone;
}

std::string EntityAttachedEMPSpawn::format() const {
	return formatString("EntityAttachedEMPSpawn") + formatString(x) + formatString(y) + formatString(time);
}

void EntityAttachedEMPSpawn::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
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
	auto name = split(formattedString, TextMarshallable::DELIMITER)[0];
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

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> EMPSpawnType::legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(time, time)
	LEGAL_CHECK_EXPRESSION(x, x)
	LEGAL_CHECK_EXPRESSION(y, y)

	return std::make_pair(status, messages);
}

void EMPSpawnType::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_FLOAT(time)
	COMPILE_EXPRESSION_FOR_FLOAT(x)
	COMPILE_EXPRESSION_FOR_FLOAT(y)
}
