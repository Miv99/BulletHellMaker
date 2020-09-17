#include <LevelPack/Item.h>

#include <LevelPack/LevelPack.h>
#include <Game/Components/PlayerTag.h>
#include <Game/Components/LevelManagerTag.h>
#include <Game/Components/HealthComponent.h>

std::shared_ptr<Item> ItemFactory::create(std::string formattedString) {
	auto name = split(formattedString, TextMarshallable::DELIMITER)[0];
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

Item::Item() {
}

Item::Item(Animatable animatable, float hitboxRadius, float activationRadius) 
	: animatable(animatable), hitboxRadius(hitboxRadius), activationRadius(activationRadius) {
}

Item::Item(Animatable animatable, float hitboxRadius, SoundSettings onCollectSound, float activationRadius) 
	: animatable(animatable), hitboxRadius(hitboxRadius), onCollectSound(onCollectSound), activationRadius(activationRadius) {
}

HealthPackItem::HealthPackItem() 
	: Item() {
}

HealthPackItem::HealthPackItem(Animatable animatable, float hitboxRadius, float activationRadius) 
	: Item(animatable, hitboxRadius, activationRadius) {
}

HealthPackItem::HealthPackItem(Animatable animatable, float hitboxRadius, SoundSettings onCollectSound, float activationRadius) 
	: Item(animatable, hitboxRadius, onCollectSound, activationRadius) {
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
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	animatable.load(items.at(1));
	hitboxRadius = std::stof(items.at(2));
	activationRadius = std::stof(items.at(3));
	onCollectSound.load(items.at(4));
	healthRestoreAmount = items.at(5);
	symbolTable.load(items.at(6));
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> HealthPackItem::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;
	
	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(healthRestoreAmount, health restore amount)

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

PowerPackItem::PowerPackItem()
	: Item() {
}

PowerPackItem::PowerPackItem(Animatable animatable, float hitboxRadius, float activationRadius)
	: Item(animatable, hitboxRadius, activationRadius) {
}

PowerPackItem::PowerPackItem(Animatable animatable, float hitboxRadius, SoundSettings onCollectSound, float activationRadius)
	: Item(animatable, hitboxRadius, onCollectSound, activationRadius) {
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
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	animatable.load(items.at(1));
	hitboxRadius = std::stof(items.at(2));
	activationRadius = std::stof(items.at(3));
	onCollectSound.load(items.at(4));
	powerAmount = items.at(5);
	pointsPerExtraPower = items.at(6);
	symbolTable.load(items.at(7));
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> PowerPackItem::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(powerAmount, power up amount)
	LEGAL_CHECK_EXPRESSION(pointsPerExtraPower, points per extra power)

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

PointsPackItem::PointsPackItem()
	: Item() {
}

PointsPackItem::PointsPackItem(Animatable animatable, float hitboxRadius, float activationRadius)
	: Item(animatable, hitboxRadius, activationRadius) {
}

PointsPackItem::PointsPackItem(Animatable animatable, float hitboxRadius, SoundSettings onCollectSound, float activationRadius)
	: Item(animatable, hitboxRadius, onCollectSound, activationRadius) {
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
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	animatable.load(items.at(1));
	hitboxRadius = std::stof(items.at(2));
	activationRadius = std::stof(items.at(3));
	onCollectSound.load(items.at(4));
	pointsAmount = items.at(5);
	symbolTable.load(items.at(6));
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> PointsPackItem::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(pointsAmount, points amount)

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

BombItem::BombItem()
	: Item() {
}

BombItem::BombItem(Animatable animatable, float hitboxRadius, float activationRadius)
	: Item(animatable, hitboxRadius, activationRadius) {
}

BombItem::BombItem(Animatable animatable, float hitboxRadius, SoundSettings onCollectSound, float activationRadius)
	: Item(animatable, hitboxRadius, onCollectSound, activationRadius) {
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
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	animatable.load(items.at(1));
	hitboxRadius = std::stof(items.at(2));
	activationRadius = std::stof(items.at(3));
	onCollectSound.load(items.at(4));
	bombsAmount = items.at(5);
	pointsPerExtraBomb = items.at(6);
	symbolTable.load(items.at(7));
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> BombItem::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	DEFINE_PARSER_AND_EXPR_FOR_LEGAL_CHECK
	LEGAL_CHECK_EXPRESSION(bombsAmount, bombs amount)
	LEGAL_CHECK_EXPRESSION(pointsPerExtraBomb, points per extra bomb)

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
