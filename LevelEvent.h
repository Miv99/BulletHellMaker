#pragma once
#include "TextMarshallable.h"
#include "LevelPackObject.h"
#include "SpriteLoader.h"
#include "LevelPack.h"
#include <entt/entt.hpp>
#include "EntityCreationQueue.h"

class LevelEvent : public TextMarshallable {
public:
	std::string format() const = 0;
	void load(std::string formattedString) = 0;

	virtual void execute(SpriteLoader& spriteLoader, LevelPack& levelPack, entt::DefaultRegistry& registry, EntityCreationQueue& queue) = 0;
};

/*
A LevelEvent that spawns enemies.
*/
class SpawnEnemiesLevelEvent : public LevelEvent {
public:
	inline SpawnEnemiesLevelEvent() {}
	inline SpawnEnemiesLevelEvent(std::vector<EnemySpawnInfo> spawnInfo) : spawnInfo(spawnInfo) {}

	std::string format() const override;
	void load(std::string formattedString) override;

	void execute(SpriteLoader& spriteLoader, LevelPack& levelPack, entt::DefaultRegistry& registry, EntityCreationQueue& queue) override;

	inline const std::vector<EnemySpawnInfo>& getSpawnInfo() { return spawnInfo; }

private:
	std::vector<EnemySpawnInfo> spawnInfo;
};

class LevelEventFactory {
public:
	static std::shared_ptr<LevelEvent> create(std::string formattedString);
};