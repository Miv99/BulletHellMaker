#include "Level.h"

Level::Level(int id) {
	this->id = id;
}

std::shared_ptr<LevelPackObject> Level::clone() const {
	auto clone = std::make_shared<Level>();
	clone->load(format());
	return clone;
}

std::string Level::format() const {
	std::string res = tos(id) + formatString(name) + tos(events.size());
	for (std::pair<std::shared_ptr<LevelEventStartCondition>, std::shared_ptr<LevelEvent>> p : events) {
		res += formatTMObject(*p.first) + formatTMObject(*p.second);
	}
	res += formatTMObject(*healthPack) + formatTMObject(*pointPack) + formatTMObject(*powerPack) + formatTMObject(*bombItem) + formatTMObject(musicSettings)
		+ formatString(backgroundFileName) + tos(backgroundScrollSpeedX) + tos(backgroundScrollSpeedY) + tos(backgroundTextureWidth)
		+ tos(backgroundTextureHeight) + tos(bossNameColor.r) + tos(bossNameColor.g) + tos(bossNameColor.b) + tos(bossNameColor.a)
		+ tos(bossHPBarColor.r) + tos(bossHPBarColor.g) + tos(bossHPBarColor.b) + tos(bossHPBarColor.a);
	res += formatTMObject(symbolTable);
	return res;
}

void Level::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	id = std::stoi(items[0]);
	name = items[1];

	events.clear();
	enemyIDCount.clear();
	int i;
	for (i = 3; i < std::stoi(items[2]) + 3;) {
		std::shared_ptr<LevelEventStartCondition> condition = LevelEventStartConditionFactory::create(items[i++]);
		std::shared_ptr<LevelEvent> event = LevelEventFactory::create(items[i++]);
		events.push_back(std::make_pair(condition, event));

		// Update enemyIDCount if possible
		std::shared_ptr<SpawnEnemiesLevelEvent> ptr = std::dynamic_pointer_cast<SpawnEnemiesLevelEvent>(event);
		if (ptr) {
			for (auto& enemy : ptr->getSpawnInfo()) {
				int enemyID = enemy->getEnemyID();
				if (enemyIDCount.count(enemyID) == 0) {
					enemyIDCount[enemyID] = 1;
				} else {
					enemyIDCount[enemyID]++;
				}
			}
		}
	}
	if (!healthPack) healthPack = std::make_shared<HealthPackItem>();
	healthPack->load(items[i++]);
	if (!pointPack) pointPack = std::make_shared<PointsPackItem>();
	pointPack->load(items[i++]);
	if (!powerPack) powerPack = std::make_shared<PowerPackItem>();
	powerPack->load(items[i++]);
	if (!bombItem) bombItem = std::make_shared<BombItem>();
	bombItem->load(items[i++]);
	musicSettings.load(items[i++]);
	backgroundFileName = items[i++];
	backgroundScrollSpeedX = std::stof(items[i++]);
	backgroundScrollSpeedY = std::stof(items[i++]);
	backgroundTextureWidth = std::stof(items[i++]);
	backgroundTextureHeight = std::stof(items[i++]);
	bossNameColor = sf::Color(std::stof(items[i++]), std::stof(items[i++]), std::stof(items[i++]), std::stof(items[i++]));
	bossHPBarColor = sf::Color(std::stof(items[i++]), std::stof(items[i++]), std::stof(items[i++]), std::stof(items[i++]));
	symbolTable.load(items[i++]);
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> Level::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	LEGAL_STATUS status = LEGAL_STATUS::LEGAL;
	std::vector<std::string> messages;

	int i = 0;
	for (std::pair<std::shared_ptr<LevelEventStartCondition>, std::shared_ptr<LevelEvent>> p : events) {
		auto eventStartConditionLegal = p.first->legal(levelPack, spriteLoader, symbolTables);
		if (eventStartConditionLegal.first != LEGAL_STATUS::LEGAL) {
			status = std::max(status, eventStartConditionLegal.first);
			tabEveryLine(eventStartConditionLegal.second);
			messages.push_back("Event index " + std::to_string(i) + ":");
			messages.insert(messages.end(), eventStartConditionLegal.second.begin(), eventStartConditionLegal.second.end());
		}

		auto eventLegal = p.second->legal(levelPack, spriteLoader, symbolTables);
		if (eventLegal.first != LEGAL_STATUS::LEGAL) {
			status = std::max(status, eventLegal.first);
			tabEveryLine(eventLegal.second);
			messages.push_back("Event index " + std::to_string(i) + ":");
			messages.insert(messages.end(), eventLegal.second.begin(), eventLegal.second.end());
		}

		i++;
	}
	// TODO: legal check healthPack, pointPack, powerPack, bombItem, musicSettings, bloomLayerSettings
	return std::make_pair(status, messages);
}

void Level::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	if (!symbolTable.isEmpty()) {
		symbolTables.push_back(symbolTable.toExprtkSymbolTable());
	}
	for (auto p : events) {
		p.second->compileExpressions(symbolTables);
	}
}

void Level::insertEvent(int eventIndex, std::shared_ptr<LevelEventStartCondition> startCondition, std::shared_ptr<LevelEvent> event) {
	events.insert(events.begin() + eventIndex, std::make_pair(startCondition, event));

	// Update enemyIDCount if possible
	std::shared_ptr<SpawnEnemiesLevelEvent> ptr = std::dynamic_pointer_cast<SpawnEnemiesLevelEvent>(event);
	if (ptr) {
		for (auto& enemy : ptr->getSpawnInfo()) {
			int enemyID = enemy->getEnemyID();
			if (enemyIDCount.count(enemyID) == 0) {
				enemyIDCount[enemyID] = 1;
			} else {
				enemyIDCount[enemyID]++;
			}
		}
	}
}

void Level::removeEvent(int eventIndex) {
	events.erase(events.begin() + eventIndex);
}
