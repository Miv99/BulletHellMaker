#pragma once
#include <entt/entt.hpp>

class LevelPack;
class Level;
class GameInstance;
class EntityCreationQueue;
class SpriteLoader;
class ShowDialogueLevelEvent;

/*
Component assigned only to a single entity - the level manager.
The level manager entity is destroyed and re-created on the start of a new level.
*/
class LevelManagerTag {
public:
	/*
	Constructor for a LevelManagerTag to be used in a GameInstance.
	*/
	LevelManagerTag(LevelPack* levelPack, std::shared_ptr<Level> level, GameInstance* gameInstance);
	/*
	Constructor for a LevelManagerTag to be used in anything else that's not a GameInstance.
	*/
	LevelManagerTag(LevelPack* levelPack, std::shared_ptr<Level> level);
	void update(EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry& registry, float deltaTime);

	void showDialogue(ShowDialogueLevelEvent* dialogueEvent);

	float getTimeSinceStartOfLevel() const;
	float getTimeSinceLastEnemySpawn() const;
	int getPoints() const;
	std::shared_ptr<Level> getLevel() const;
	LevelPack* getLevelPack() const;
	std::shared_ptr<entt::SigH<void(int)>> getPointsChangeSignal();
	std::shared_ptr<entt::SigH<void(uint32_t)>> getEnemySpawnSignal();

	/*
	Should be called whenever an enemy is spawned.

	enemy - the enemy entity
	*/
	void onEnemySpawn(uint32_t enemy);

	void addPoints(int amount);
	void subtractPoints(int amount);

private:
	LevelPack* levelPack;
	// The GameInstance this component's entity belongs to, if any. May be nullptr.
	GameInstance* gameInstance;

	// Time since the start of the level
	float timeSinceStartOfLevel = 0;
	// Time since the last enemy spawn
	// If no enemies have been spawned yet, this is the same as timeSinceStartOfLevel
	float timeSinceLastEnemySpawn = 0;

	std::shared_ptr<Level> level;
	// Current index in list of LevelEvents
	int currentLevelEventsIndex = -1;

	// Points earned so far
	int points = 0;

	// function accepts 1 int: number of points from the current level so far
	std::shared_ptr<entt::SigH<void(int)>> pointsChangeSignal;
	// function accepts 1 int: the enemy entity id that just spawned
	std::shared_ptr<entt::SigH<void(uint32_t)>> enemySpawnSignal;

	/*
	Called whenever points changes.
	*/
	void onPointsChange();
};