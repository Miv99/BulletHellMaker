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

std::shared_ptr<LevelEvent> LevelEventFactory::create(std::string formattedString) {
	auto name = split(formattedString, DELIMITER)[0];
	std::shared_ptr<LevelEvent> ptr;
	if (name == "SpawnEnemiesLevelEvent") {
		ptr = std::make_shared<SpawnEnemiesLevelEvent>();
	}
	ptr->load(formattedString);
	return ptr;
}
