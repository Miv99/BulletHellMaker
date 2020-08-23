#include <Game/Components/LevelManagerTag.h>

#include <DataStructs/SpriteLoader.h>
#include <LevelPack/LevelPack.h>
#include <LevelPack/LevelEvent.h>
#include <LevelPack/Level.h>
#include <Game/EntityCreationQueue.h>
#include <Game/GameInstance.h>

LevelManagerTag::LevelManagerTag(LevelPack* levelPack, std::shared_ptr<Level> level, GameInstance* gameInstance) : levelPack(levelPack), level(level), gameInstance(gameInstance) {
}

LevelManagerTag::LevelManagerTag(LevelPack* levelPack, std::shared_ptr<Level> level) : levelPack(levelPack), level(level) {
}

void LevelManagerTag::showDialogue(ShowDialogueLevelEvent* dialogueEvent) {
	// Only show dialogue when playing a game using GameInstance
	if (gameInstance) {
		gameInstance->showDialogue(dialogueEvent);
	}
}

void LevelManagerTag::update(EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry& registry, float deltaTime) {
	timeSinceStartOfLevel += deltaTime;
	timeSinceLastEnemySpawn += deltaTime;

	while (currentLevelEventsIndex + 1 < level->getEventsCount()) {
		if (level->conditionSatisfied(currentLevelEventsIndex + 1, registry)) {
			level->executeEvent(currentLevelEventsIndex + 1, spriteLoader, *levelPack, registry, queue);
			currentLevelEventsIndex++;
		} else {
			break;
		}
	}
}

float LevelManagerTag::getTimeSinceStartOfLevel() const {
	return timeSinceStartOfLevel;
}

float LevelManagerTag::getTimeSinceLastEnemySpawn() const {
	return timeSinceLastEnemySpawn;
}

int LevelManagerTag::getPoints() const {
	return points;
}

std::shared_ptr<Level> LevelManagerTag::getLevel() const {
	return level;
}

LevelPack* LevelManagerTag::getLevelPack() const {
	return levelPack;
}

std::shared_ptr<entt::SigH<void(int)>> LevelManagerTag::getPointsChangeSignal() {
	if (pointsChangeSignal) {
		return pointsChangeSignal;
	}
	pointsChangeSignal = std::make_shared<entt::SigH<void(int)>>();
	return pointsChangeSignal;
}

std::shared_ptr<entt::SigH<void(uint32_t)>> LevelManagerTag::getEnemySpawnSignal() {
	if (enemySpawnSignal) {
		return enemySpawnSignal;
	}
	enemySpawnSignal = std::make_shared<entt::SigH<void(uint32_t)>>();
	return enemySpawnSignal;
}

void LevelManagerTag::onEnemySpawn(uint32_t enemy) {
	timeSinceLastEnemySpawn = 0;
	if (enemySpawnSignal) {
		enemySpawnSignal->publish(enemy);
	}
}

void LevelManagerTag::addPoints(int amount) {
	points += amount;
	onPointsChange();
}

void LevelManagerTag::subtractPoints(int amount) {
	points -= amount;
	if (points < 0) points = 0;
	onPointsChange();
}

void LevelManagerTag::onPointsChange() {
	if (pointsChangeSignal) {
		pointsChangeSignal->publish(points);
	}
}