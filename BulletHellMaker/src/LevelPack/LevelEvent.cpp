#include <LevelPack/LevelEvent.h>

SpawnEnemiesLevelEvent::SpawnEnemiesLevelEvent() {
}

SpawnEnemiesLevelEvent::SpawnEnemiesLevelEvent(std::vector<std::shared_ptr<EnemySpawnInfo>> spawnInfo)
	: spawnInfo(spawnInfo) {
}

std::string SpawnEnemiesLevelEvent::format() const {
	std::string res = formatString("SpawnEnemiesLevelEvent") + tos(spawnInfo.size()) + formatTMObject(symbolTable);
	for (auto info : spawnInfo) {
		res += formatTMObject(*info);
	}
	return res;
}

void SpawnEnemiesLevelEvent::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	int numInfo = std::stoi(items.at(1));
	symbolTable.load(items.at(2));
	spawnInfo.clear();
	for (int i = 3; i < numInfo + 3; i++) {
		std::shared_ptr<EnemySpawnInfo> info = std::make_shared<EnemySpawnInfo>();
		info->load(items.at(i));
		spawnInfo.push_back(info);
	}
}

nlohmann::json SpawnEnemiesLevelEvent::toJson() {
	nlohmann::json j = {
		{"className", "SpawnEnemiesLevelEvent"},
		{"valueSymbolTable", symbolTable.toJson()}
	};

	nlohmann::json spawnInfoJson;
	for (auto item : spawnInfo) {
		spawnInfoJson.push_back(item->toJson());
	}
	j["spawnInfo"] = spawnInfoJson;

	return j;
}

void SpawnEnemiesLevelEvent::load(const nlohmann::json& j) {
	symbolTable.load(j.at("valueSymbolTable"));

	spawnInfo.clear();
	if (j.contains("spawnInfo")) {
		for (const nlohmann::json& item : j.at("spawnInfo")) {
			std::shared_ptr<EnemySpawnInfo> info = std::make_shared<EnemySpawnInfo>();
			info->load(item);
			spawnInfo.push_back(info);
		}
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

ShowDialogueLevelEvent::ShowDialogueLevelEvent() {
}

ShowDialogueLevelEvent::ShowDialogueLevelEvent(std::string dialogueBoxTextureFileName, std::vector<std::string> text, POSITION_ON_SCREEN pos, tgui::ShowAnimationType showAnimation)
	: text(text), dialogueBoxPosition(pos), dialogueBoxShowAnimationType(showAnimation), dialogueBoxTextureFileName(dialogueBoxTextureFileName) {
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
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	dialogueBoxPosition = static_cast<POSITION_ON_SCREEN>(std::stoi(items.at(1)));
	dialogueBoxShowAnimationType = static_cast<tgui::ShowAnimationType>(std::stoi(items.at(2)));
	dialogueBoxShowAnimationTime = std::stof(items.at(3));
	dialogueBoxTextureFileName = items.at(4);
	textureMiddlePart = sf::IntRect(std::stoi(items.at(5)), std::stoi(items.at(6)), std::stoi(items.at(7)), std::stoi(items.at(8)));
	dialogueBoxPortraitFileName = items.at(9);
	symbolTable.load(items.at(10));
	text.clear();
	for (int i = 11; i < items.size() + 11; i++) {
		text.push_back(items.at(i));
	}
}

nlohmann::json ShowDialogueLevelEvent::toJson() {
	nlohmann::json j = {
		{"className", "ShowDialogueLevelEvent"},
		{"dialogueBoxPosition", dialogueBoxPosition},
		{"dialogueBoxShowAnimationType", dialogueBoxShowAnimationType},
		{"dialogueBoxShowAnimationTime", dialogueBoxShowAnimationTime},
		{"dialogueBoxTextureFileName", dialogueBoxTextureFileName},
		{"textureMiddlePart", nlohmann::json{ {"left", textureMiddlePart.left}, {"top", textureMiddlePart.top},
			{"width", textureMiddlePart.width}, {"height", textureMiddlePart.height} }},
		{"dialogueBoxPortraitFileName", dialogueBoxPortraitFileName}
	};

	nlohmann::json textJson;
	for (std::string str : text) {
		textJson.push_back(str);
	}
	j["text"] = textJson;

	return j;
}

void ShowDialogueLevelEvent::load(const nlohmann::json& j) {
	j.at("dialogueBoxPosition").get_to(dialogueBoxPosition);
	j.at("dialogueBoxShowAnimationType").get_to(dialogueBoxShowAnimationType);
	j.at("dialogueBoxShowAnimationTime").get_to(dialogueBoxShowAnimationTime);
	j.at("dialogueBoxTextureFileName").get_to(dialogueBoxTextureFileName);
	j.at("textureMiddlePart").at("left").get_to(textureMiddlePart.left);
	j.at("textureMiddlePart").at("top").get_to(textureMiddlePart.top);
	j.at("textureMiddlePart").at("width").get_to(textureMiddlePart.width);
	j.at("textureMiddlePart").at("height").get_to(textureMiddlePart.height);
	j.at("dialogueBoxPortraitFileName").get_to(dialogueBoxPortraitFileName);

	text.clear();
	if (j.contains("text")) {
		for (const nlohmann::json& item : j.at("text")) {
			std::string str;
			item.get_to(str);
			text.push_back(str);
		}
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
	auto name = split(formattedString, TextMarshallable::DELIMITER)[0];
	std::shared_ptr<LevelEvent> ptr;
	if (name == "SpawnEnemiesLevelEvent") {
		ptr = std::make_shared<SpawnEnemiesLevelEvent>();
	} else if (name == "ShowDialogueLevelEvent") {
		ptr = std::make_shared<ShowDialogueLevelEvent>();
	}
	ptr->load(formattedString);
	return ptr;
}

std::shared_ptr<LevelEvent> LevelEventFactory::create(const nlohmann::json& j) {
	if (j.contains("className")) {
		std::string name;
		j.at("className").get_to(name);

		std::shared_ptr<LevelEvent> ptr;
		if (name == "SpawnEnemiesLevelEvent") {
			ptr = std::make_shared<SpawnEnemiesLevelEvent>();
		} else if (name == "ShowDialogueLevelEvent") {
			ptr = std::make_shared<ShowDialogueLevelEvent>();
		}
		ptr->load(j);
		return ptr;
	} else {
		return std::make_shared<SpawnEnemiesLevelEvent>();
	}
}
