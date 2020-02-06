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

std::string HealthPackItem::format() const {
	return formatString("HealthPackItem") + formatTMObject(animatable) + tos(hitboxRadius) + tos(activationRadius) + formatTMObject(onCollectSound);
}

void HealthPackItem::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	animatable.load(items[1]);
	hitboxRadius = std::stof(items[2]);
	activationRadius = std::stof(items[3]);
	onCollectSound.load(items[4]);
}

void HealthPackItem::onPlayerContact(entt::DefaultRegistry & registry, uint32_t player) {
	Item::onPlayerContact(registry, player);
	registry.get<HealthComponent>(player).heal(HEALTH_PER_HEALTH_PACK);
}

std::string PowerPackItem::format() const {
	return formatString("PowerPackItem") + formatTMObject(animatable) + tos(hitboxRadius) + tos(activationRadius) + formatTMObject(onCollectSound);
}

void PowerPackItem::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	animatable.load(items[1]);
	hitboxRadius = std::stof(items[2]);
	activationRadius = std::stof(items[3]);
	onCollectSound.load(items[4]);
}

void PowerPackItem::onPlayerContact(entt::DefaultRegistry & registry, uint32_t player) {
	Item::onPlayerContact(registry, player);
	registry.get<PlayerTag>().increasePower(registry, player, POWER_PER_POWER_PACK);
}

std::string PointsPackItem::format() const {
	return formatString("PointsPackItem") + formatTMObject(animatable) + tos(hitboxRadius) + tos(activationRadius) + formatTMObject(onCollectSound);
}

void PointsPackItem::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	animatable.load(items[1]);
	hitboxRadius = std::stof(items[2]);
	activationRadius = std::stof(items[3]);
	onCollectSound.load(items[4]);
}

void PointsPackItem::onPlayerContact(entt::DefaultRegistry & registry, uint32_t player) {
	Item::onPlayerContact(registry, player);
	registry.get<LevelManagerTag>().addPoints(POINTS_PER_POINTS_PACK);
}

std::string BombItem::format() const {
	return formatString("BombItem") + formatTMObject(animatable) + tos(hitboxRadius) + tos(activationRadius) + formatTMObject(onCollectSound);
}

void BombItem::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	animatable.load(items[1]);
	hitboxRadius = std::stof(items[2]);
	activationRadius = std::stof(items[3]);
	onCollectSound.load(items[4]);
}

void BombItem::onPlayerContact(entt::DefaultRegistry & registry, uint32_t player) {
	Item::onPlayerContact(registry, player);
	registry.get<PlayerTag>().gainBombs(registry, 1);
}

void Item::onPlayerContact(entt::DefaultRegistry & registry, uint32_t player) {
	registry.get<LevelManagerTag>().getLevelPack()->playSound(onCollectSound);
}
