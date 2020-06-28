#include "Item.h"
#include "LevelPack.h"

std::shared_ptr<Item> ItemFactory::create(std::string formattedString) {
	auto name = split(formattedString, DELIMITER)[0];
	std::shared_ptr<Item> ptr;
	if (name == "HealthPackItem") {
		ptr = std::make_shared<HealthPackItem>();
	} else if (name == "PowerPackItem") {
		ptr = std::make_shared<PowerPackItem>();
	} else if (name == "BombItem") {
		ptr = std::make_shared<BombItem>();
	} else if (name == "PointsPackItem") {
		ptr = std::make_shared<PointsPackItem>();
	}
	ptr->load(formattedString);
	return ptr;
}

std::shared_ptr<LevelPackObject> HealthPackItem::clone() const {
	auto clone = std::make_shared<HealthPackItem>();
	clone->load(format());
	return clone;
}

std::string HealthPackItem::format() const {
	return formatString("HealthPackItem") + formatTMObject(animatable) + tos(hitboxRadius) + tos(activationRadius) + formatTMObject(onCollectSound)
		+ formatString(healthRestoreAmount) + formatTMObject(symbolTable);
}

void HealthPackItem::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	animatable.load(items[1]);
	hitboxRadius = std::stof(items[2]);
	activationRadius = std::stof(items[3]);
	onCollectSound.load(items[4]);
	healthRestoreAmount = items[5];
	symbolTable.load(items[6]);
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> HealthPackItem::legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;
	exprtk::parser<float> parser;
	if (!expressionStrIsValid(parser, healthRestoreAmount, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for health restore amount");
	}
	return std::make_pair(status, messages);
}

void HealthPackItem::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_INT(healthRestoreAmount)
}

void HealthPackItem::onPlayerContact(entt::DefaultRegistry & registry, uint32_t player) {
	Item::onPlayerContact(registry, player);
	registry.get<HealthComponent>(player).heal(healthRestoreAmountExprCompiledValue);
}

std::shared_ptr<LevelPackObject> PowerPackItem::clone() const {
	auto clone = std::make_shared<PowerPackItem>();
	clone->load(format());
	return clone;
}

std::string PowerPackItem::format() const {
	return formatString("PowerPackItem") + formatTMObject(animatable) + tos(hitboxRadius) + tos(activationRadius) + formatTMObject(onCollectSound)
		+ formatString(powerAmount) + formatString(pointsPerExtraPower) + formatTMObject(symbolTable);
}

void PowerPackItem::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	animatable.load(items[1]);
	hitboxRadius = std::stof(items[2]);
	activationRadius = std::stof(items[3]);
	onCollectSound.load(items[4]);
	powerAmount = items[5];
	pointsPerExtraPower = items[6];
	symbolTable.load(items[7]);
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> PowerPackItem::legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;
	exprtk::parser<float> parser;
	if (!expressionStrIsValid(parser, powerAmount, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for power up amount");
	}
	if (!expressionStrIsValid(parser, pointsPerExtraPower, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for points per extra power");
	}
	return std::make_pair(status, messages);
}

void PowerPackItem::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_INT(powerAmount)
	COMPILE_EXPRESSION_FOR_INT(pointsPerExtraPower)
}

void PowerPackItem::onPlayerContact(entt::DefaultRegistry & registry, uint32_t player) {
	Item::onPlayerContact(registry, player);
	registry.get<PlayerTag>().increasePower(registry, player, powerAmountExprCompiledValue, pointsPerExtraPowerExprCompiledValue);
}

std::shared_ptr<LevelPackObject> PointsPackItem::clone() const {
	auto clone = std::make_shared<PointsPackItem>();
	clone->load(format());
	return clone;
}

std::string PointsPackItem::format() const {
	return formatString("PointsPackItem") + formatTMObject(animatable) + tos(hitboxRadius) + tos(activationRadius) + formatTMObject(onCollectSound)
		+ formatString(pointsAmount) + formatTMObject(symbolTable);
}

void PointsPackItem::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	animatable.load(items[1]);
	hitboxRadius = std::stof(items[2]);
	activationRadius = std::stof(items[3]);
	onCollectSound.load(items[4]);
	pointsAmount = items[5];
	symbolTable.load(items[6]);
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> PointsPackItem::legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;
	exprtk::parser<float> parser;
	if (!expressionStrIsValid(parser, pointsAmount, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for points amount");
	}
	return std::make_pair(status, messages);
}

void PointsPackItem::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_INT(pointsAmount)
}

void PointsPackItem::onPlayerContact(entt::DefaultRegistry & registry, uint32_t player) {
	Item::onPlayerContact(registry, player);
	registry.get<LevelManagerTag>().addPoints(pointsAmountExprCompiledValue);
}

std::shared_ptr<LevelPackObject> BombItem::clone() const {
	auto clone = std::make_shared<BombItem>();
	clone->load(format());
	return clone;
}

std::string BombItem::format() const {
	return formatString("BombItem") + formatTMObject(animatable) + tos(hitboxRadius) + tos(activationRadius) + formatTMObject(onCollectSound)
		+ formatString(bombsAmount) + formatString(pointsPerExtraBomb) + formatTMObject(symbolTable);
}

void BombItem::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	animatable.load(items[1]);
	hitboxRadius = std::stof(items[2]);
	activationRadius = std::stof(items[3]);
	onCollectSound.load(items[4]);
	bombsAmount = items[5];
	pointsPerExtraBomb = items[6];
	symbolTable.load(items[7]);
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> BombItem::legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;
	exprtk::parser<float> parser;
	if (!expressionStrIsValid(parser, bombsAmount, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for bombs amount");
	}
	if (!expressionStrIsValid(parser, pointsPerExtraBomb, symbolTable)) {
		status = std::max(status, LEGAL_STATUS::ILLEGAL);
		messages.push_back("Invalid expression for points per extra bomb");
	}
	return std::make_pair(status, messages);
}

void BombItem::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	DEFINE_PARSER_AND_EXPR_FOR_COMPILE
	COMPILE_EXPRESSION_FOR_INT(bombsAmount)
	COMPILE_EXPRESSION_FOR_INT(pointsPerExtraBomb)
}

void BombItem::onPlayerContact(entt::DefaultRegistry & registry, uint32_t player) {
	Item::onPlayerContact(registry, player);
	registry.get<PlayerTag>().gainBombs(registry, bombsAmountExprCompiledValue, pointsPerExtraBombExprCompiledValue);
}

void Item::onPlayerContact(entt::DefaultRegistry & registry, uint32_t player) {
	registry.get<LevelManagerTag>().getLevelPack()->playSound(onCollectSound);
}
