#include <LevelPack/Level.h>

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
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	id = std::stoi(items.at(0));
	name = items.at(1);

	events.clear();
	enemyIDCount.clear();
	int i;
	for (i = 3; i < std::stoi(items.at(2)) + 3;) {
		std::shared_ptr<LevelEventStartCondition> condition = LevelEventStartConditionFactory::create(items.at(i++));
		std::shared_ptr<LevelEvent> event = LevelEventFactory::create(items.at(i++));
		events.push_back(std::make_pair(condition, event));

		// Update enemyIDCount if possible
		std::shared_ptr<SpawnEnemiesLevelEvent> ptr = std::dynamic_pointer_cast<SpawnEnemiesLevelEvent>(event);
		if (ptr) {
			for (auto& enemy : ptr->getSpawnInfo()) {
				int enemyID = enemy->getEnemyID();
				if (enemyIDCount.find(enemyID) == enemyIDCount.end()) {
					enemyIDCount[enemyID] = 1;
				} else {
					enemyIDCount[enemyID]++;
				}
			}
		}
	}
	if (!healthPack) healthPack = std::make_shared<HealthPackItem>();
	healthPack->load(items.at(i++));
	if (!pointPack) pointPack = std::make_shared<PointsPackItem>();
	pointPack->load(items.at(i++));
	if (!powerPack) powerPack = std::make_shared<PowerPackItem>();
	powerPack->load(items.at(i++));
	if (!bombItem) bombItem = std::make_shared<BombItem>();
	bombItem->load(items.at(i++));
	musicSettings.load(items.at(i++));
	backgroundFileName = items.at(i++);
	backgroundScrollSpeedX = std::stof(items.at(i++));
	backgroundScrollSpeedY = std::stof(items.at(i++));
	backgroundTextureWidth = std::stof(items.at(i++));
	backgroundTextureHeight = std::stof(items.at(i++));
	bossNameColor = sf::Color(std::stof(items.at(i++)), std::stof(items.at(i++)), std::stof(items.at(i++)), std::stof(items.at(i++)));
	bossHPBarColor = sf::Color(std::stof(items.at(i++)), std::stof(items.at(i++)), std::stof(items.at(i++)), std::stof(items.at(i++)));
	symbolTable.load(items.at(i++));
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

std::shared_ptr<HealthPackItem> Level::getHealthPack() {
	if (!healthPack) {
		healthPack = std::make_shared<HealthPackItem>();
	}
	return healthPack;
}

std::shared_ptr<PointsPackItem> Level::getPointsPack() {
	if (!pointPack) {
		pointPack = std::make_shared<PointsPackItem>();
	}
	return pointPack;
}

std::shared_ptr<PowerPackItem> Level::getPowerPack() {
	if (!powerPack) {
		powerPack = std::make_shared<PowerPackItem>();
	}
	return powerPack;
}

std::shared_ptr<BombItem> Level::getBombItem() {
	if (!bombItem) {
		bombItem = std::make_shared<BombItem>();
	}
	return bombItem;
}

void Level::insertEvent(int eventIndex, std::shared_ptr<LevelEventStartCondition> startCondition, std::shared_ptr<LevelEvent> event) {
	events.insert(events.begin() + eventIndex, std::make_pair(startCondition, event));

	// Update enemyIDCount if possible
	std::shared_ptr<SpawnEnemiesLevelEvent> ptr = std::dynamic_pointer_cast<SpawnEnemiesLevelEvent>(event);
	if (ptr) {
		for (auto& enemy : ptr->getSpawnInfo()) {
			int enemyID = enemy->getEnemyID();
			if (enemyIDCount.find(enemyID) == enemyIDCount.end()) {
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
