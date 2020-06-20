#include "LevelEvent.h"

std::string SpawnEnemiesLevelEvent::format() const {
	std::string res = formatString("SpawnEnemiesLevelEvent") + tos(spawnInfo.size());
	for (auto& info : spawnInfo) {
		res += formatTMObject(info);
	}
	return res;
}

void SpawnEnemiesLevelEvent::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	int numInfo = std::stoi(items[1]);
	spawnInfo.clear();
	for (int i = 2; i < numInfo + 2; i++) {
		EnemySpawnInfo info;
		info.load(items[i]);
		spawnInfo.push_back(info);
	}
}

void SpawnEnemiesLevelEvent::execute(SpriteLoader & spriteLoader, LevelPack & levelPack, entt::DefaultRegistry & registry, EntityCreationQueue & queue) {
	for (EnemySpawnInfo info : spawnInfo) {
		info.spawnEnemy(spriteLoader, levelPack, registry, queue);
	}
}

std::string ShowDialogueLevelEvent::format() const {
	std::string res = formatString("ShowDialogueLevelEvent") + tos(static_cast<int>(dialogueBoxPosition)) + tos(static_cast<int>(dialogueBoxShowAnimationType)) + tos(dialogueBoxShowAnimationTime)
		+ formatString(dialogueBoxTextureFileName) + tos(textureMiddlePart.left) + tos(textureMiddlePart.top) + tos(textureMiddlePart.width) + tos(textureMiddlePart.height) + formatString(dialogueBoxPortraitFileName);
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
	text.clear();
	for (int i = 10; i < items.size() + 10; i++) {
		text.push_back(items[i]);
	}
}

void ShowDialogueLevelEvent::execute(SpriteLoader& spriteLoader, LevelPack& levelPack, entt::DefaultRegistry& registry, EntityCreationQueue& queue) {
	//registry.get<LevelManagerTag>().showDialogue(this);
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
