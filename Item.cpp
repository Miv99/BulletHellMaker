#include "Item.h"

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

std::string HealthPackItem::format() {
	return "HealthPackItem" + delim + "(" + animatable.format() + ")" + delim + tos(hitboxRadius) + delim + tos(activationRadius);
}

void HealthPackItem::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	animatable.load(items[1]);
	hitboxRadius = std::stof(items[2]);
	activationRadius = std::stof(items[3]);
}

std::string PowerPackItem::format() {
	return "PowerPackItem" + delim + "(" + animatable.format() + ")" + delim + tos(hitboxRadius) + delim + tos(activationRadius);
}

void PowerPackItem::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	animatable.load(items[1]);
	hitboxRadius = std::stof(items[2]);
	activationRadius = std::stof(items[3]);
}

std::string PointsPackItem::format() {
	return "PointsPackItem" + delim + "(" + animatable.format() + ")" + delim + tos(hitboxRadius) + delim + tos(activationRadius);
}

void PointsPackItem::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	animatable.load(items[1]);
	hitboxRadius = std::stof(items[2]);
	activationRadius = std::stof(items[3]);
}

std::string BombItem::format() {
	return "BombItem" + delim + "(" + animatable.format() + ")" + delim + tos(hitboxRadius) + delim + tos(activationRadius);
}

void BombItem::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	animatable.load(items[1]);
	hitboxRadius = std::stof(items[2]);
	activationRadius = std::stof(items[3]);
}