#include <LevelPack/LevelEvent.h>

std::string SpawnEnemiesLevelEvent::format() const {
	std::string res = formatString("SpawnEnemiesLevelEvent") + tos(spawnInfo.size()) + formatTMObject(symbolTable);
	for (auto info : spawnInfo) {
		res += formatTMObject(*info);
	}
	return res;
}

void SpawnEnemiesLevelEvent::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	int numInfo = std::stoi(items[1]);
	symbolTable.load(items[2]);
	spawnInfo.clear();
	for (int i = 3; i < numInfo + 3; i++) {
		std::shared_ptr<EnemySpawnInfo> info = std::make_shared<EnemySpawnInfo>();
		info->load(items[i]);
		spawnInfo.push_back(info);
	}
}

std::shared_ptr<LevelPackObject> SpawnEnemiesLevelEvent::clone() const {
	auto clone = std::make_shared<SpawnEnemiesLevelEvent>();
	clone->load(format());
	return clone;
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> SpawnEnemiesLevelEvent::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	std::vector<std::string> messages;
	if (spawnInfo.size() == 0) {
		messages.push_back("Missing enemy spawn information.");
		return std::make_pair(LEGAL_STATUS::ILLEGAL, messages);
	} else {
		LEGAL_STATUS status = LEGAL_STATUS::LEGAL;

		int i = 0;
		for (std::shared_ptr<EnemySpawnInfo> info : spawnInfo) {
			auto infoLegal = info->legal(levelPack, spriteLoader, symbolTables);
			if (infoLegal.first != LEGAL_STATUS::LEGAL) {
				status = std::max(status, infoLegal.first);
				tabEveryLine(infoLegal.second);
				messages.push_back("Enemy spawn info index " + std::to_string(i) + ":");
				messages.insert(messages.end(), infoLegal.second.begin(), infoLegal.second.end());
			}
			i++;
		}
		return std::make_pair(status, messages);
	}
}

void SpawnEnemiesLevelEvent::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	if (!symbolTable.isEmpty()) {
		symbolTables.push_back(symbolTable.toExprtkSymbolTable());
	}

	for (std::shared_ptr<EnemySpawnInfo> info : spawnInfo) {
		info->compileExpressions(symbolTables);
	}
}

void SpawnEnemiesLevelEvent::execute(SpriteLoader & spriteLoader, LevelPack & levelPack, entt::DefaultRegistry & registry, EntityCreationQueue & queue) {
	for (std::shared_ptr<EnemySpawnInfo> info : spawnInfo) {
		info->spawnEnemy(spriteLoader, levelPack, registry, queue);
	}
}

std::string ShowDialogueLevelEvent::format() const {
	std::string res = formatString("ShowDialogueLevelEvent") + tos(static_cast<int>(dialogueBoxPosition)) + tos(static_cast<int>(dialogueBoxShowAnimationType)) + tos(dialogueBoxShowAnimationTime)
		+ formatString(dialogueBoxTextureFileName) + tos(textureMiddlePart.left) + tos(textureMiddlePart.top) + tos(textureMiddlePart.width) + tos(textureMiddlePart.height) 
		+ formatString(dialogueBoxPortraitFileName) + formatTMObject(symbolTable);
	for (std::string str : text) {
		res += formatString(str);
	}
	return res;
}

void ShowDialogueLevelEvent::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	dialogueBoxPosition = static_cast<PositionOnScreen>(std::stoi(items[1]));
	dialogueBoxShowAnimationType = static_cast<tgui::ShowAnimationType>(std::stoi(items[2]));
	dialogueBoxShowAnimationTime = std::stof(items[3]);
	dialogueBoxTextureFileName = items[4];
	textureMiddlePart = sf::IntRect(std::stoi(items[5]), std::stoi(items[6]), std::stoi(items[7]), std::stoi(items[8]));
	dialogueBoxPortraitFileName = items[9];
	symbolTable.load(items[10]);
	text.clear();
	for (int i = 11; i < items.size() + 11; i++) {
		text.push_back(items[i]);
	}
}

std::shared_ptr<LevelPackObject> ShowDialogueLevelEvent::clone() const {
	auto clone = std::make_shared<SpawnEnemiesLevelEvent>();
	clone->load(format());
	return clone;
}

std::pair<LevelPackObject::LEGAL_STATUS, std::vector<std::string>> ShowDialogueLevelEvent::legal(LevelPack & levelPack, SpriteLoader & spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const {
	//TODO: legal
	return std::make_pair(LEGAL_STATUS::ILLEGAL, std::vector<std::string>());
}

void ShowDialogueLevelEvent::compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) {
	// Nothing needs to be done
}

void ShowDialogueLevelEvent::execute(SpriteLoader& spriteLoader, LevelPack& levelPack, entt::DefaultRegistry& registry, EntityCreationQueue& queue) {
	registry.get<LevelManagerTag>().showDialogue(this);
}

std::shared_ptr<LevelEvent> LevelEventFactory::create(std::string formattedString) {
	auto name = split(formattedString, DELIMITER)[0];
	std::shared_ptr<LevelEvent> ptr;
	if (name == "SpawnEnemiesLevelEvent") {
		ptr = std::make_shared<SpawnEnemiesLevelEvent>();
	} else if (name == "ShowDialogueLevelEvent") {
		ptr = std::make_shared<ShowDialogueLevelEvent>();
	}
	ptr->load(formattedString);
	return ptr;
}