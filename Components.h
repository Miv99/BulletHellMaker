#pragma once
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <memory>
#include <queue>
#include <vector>
#include <cassert>
#include <string>
#include "SpriteLoader.h"
#include "SpriteAnimation.h"

class MovablePoint;
class AggregatorMP;
class LevelPack;
class EMPAction;
class EditorAttackPattern;
class EditorEnemyPhase;
class EditorEnemy;
class EMPSpawnType;
class Level;
class EntityCreationQueue;
class EditorMovablePoint;

class PositionComponent {
public:
	PositionComponent(float x = 0, float y = 0) : x(x), y(y) {}

	inline float getX() const { return x; }
	inline float getY() const { return y; }
	inline void setX(float x) { this->x = x; }
	inline void setY(float y) { this->y = y; }
	void setPosition(sf::Vector2f position) { x = position.x; y = position.y; }

private:
	float x;
	float y;
};

class MovementPathComponent {
public:
	/*
	entity - the entity that the entity with this component should be attached to, if any
	*/
	inline MovementPathComponent(const entt::DefaultRegistry& registry, uint32_t entity, std::shared_ptr<EMPSpawnType> spawnType, std::vector<std::shared_ptr<EMPAction>> actions, float initalTime) : actions(actions), time(initalTime) {
		initialSpawn(registry, entity, spawnType, actions);
	}

	// Update elapsed time and return the entity's new position along its path
	sf::Vector2f update(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float deltaTime);

	bool usesReferenceEntity() { return useReferenceEntity; }
	uint32_t getReferenceEntity() const { return referenceEntity; }
	void setActions(std::vector<std::shared_ptr<EMPAction>> actions);
	/*
	Sets the reference entity of this entity.
	The PositionComponent of this entity should be updated after this call.
	*/
	inline void setReferenceEntity(uint32_t reference) {
		useReferenceEntity = true;
		referenceEntity = reference;
	}
	inline std::shared_ptr<MovablePoint> getPath() { return path; }
	inline float getTime() { return time; }

private:
	bool useReferenceEntity;
	uint32_t referenceEntity;
	// Elapsed time since the the last call to setActions
	float time;
	std::shared_ptr<MovablePoint> path;
	// Actions to be carried out in order; each one changes pushes back an MP to path
	std::vector<std::shared_ptr<EMPAction>> actions;
	int currentActionsIndex = 0;

	void initialSpawn(const entt::DefaultRegistry& registry, uint32_t entity, std::shared_ptr<EMPSpawnType> spawnType, std::vector<std::shared_ptr<EMPAction>>& actions);
};

class HealthComponent {
public:
	HealthComponent(float health = 0, float maxHealth = 0) : health(health), maxHealth(maxHealth) {}

	inline float getHealth() const { return health; }
	inline float getMaxHealth() const { return maxHealth; }
	inline void setHealth(float health) { this->health = health; }
	inline void setMaxHealth(float maxHealth) { this->maxHealth = maxHealth; }

private:
	float health;
	float maxHealth;
};

/*
Hitbox is a single circle
*/
class HitboxComponent {
public:
	HitboxComponent(float radius) : radius(radius) {}

	inline float getRadius() const { return radius; }
	inline void setRadius(float radius) { this->radius = radius; }

private:
	float radius;
};

class SpriteComponent {
public:
	SpriteComponent(std::shared_ptr<sf::Sprite> sprite) : sprite(sprite) {}

	void update(float deltaTime);

	inline const std::shared_ptr<sf::Sprite> getSprite() { return sprite; }
	inline void setEffectAnimation(std::unique_ptr<SpriteEffectAnimation> animation) { this->animation = std::move(animation); }
	// Angle in degrees
	inline void setRotation(float angle) { sprite->setRotation(angle); }
	inline void setScale(float x, float y) { sprite->setScale(x, y); }
	inline bool usesShader() {
		if (animation == nullptr) {
			return false;
		}
		return animation->usesShader();
	}
	inline sf::Shader& getShader() {
		assert(animation != nullptr);
		return animation->getShader();
	}

private:
	std::shared_ptr<sf::Sprite> sprite;
	// Animation that the sprite is currently undergoing, if any
	std::unique_ptr<SpriteEffectAnimation> animation;
};

class PlayerTag {};

class EnemyComponent {
public:
	EnemyComponent(std::shared_ptr<EditorEnemy> enemyData, int enemyID);
	void update(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, uint32_t entity, float deltaTime);

	inline float getTimeSinceSpawned() const { return timeSinceSpawned; }

private:
	// Time since being spawned
	float timeSinceSpawned = 0;
	// Time since start of the current phase
	float timeSincePhase = 0;
	// Time since start of the current attack pattern
	float timeSinceAttackPattern = 0;

	int enemyID;
	std::shared_ptr<EditorEnemy> enemyData;
	std::shared_ptr<EditorEnemyPhase> currentPhase = nullptr;
	std::shared_ptr<EditorAttackPattern> currentAttackPattern = nullptr;

	// Current phase index in list of phases in EditorEnemy
	int currentPhaseIndex = -1;
	// Current attack pattern index in list of attack patterns in current EditorEnemyPhase
	int currentAttackPatternIndex = -1;
	// Current attack index in list of attacks in current EditorAttackPattern
	int currentAttackIndex = -1;

	// Check for any missed phases/attack patterns/attacks since the last update and then executes their relevant actions
	void checkPhases(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, uint32_t entity);
	void checkAttackPatterns(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, uint32_t entity);
	void checkAttacks(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, uint32_t entity);
};

class DespawnComponent {
public:
	/*
	maxTime - time before entity with this component despawns
	*/
	inline DespawnComponent(float maxTime) : maxTime(maxTime), useTime(true) {}
	/*
	entity - the entity that, when it despawns, the entity with this component will despawn
	identifier - for method signature differentiation; value not important
	*/
	inline DespawnComponent(uint32_t entity, bool identifier) : attachedTo(entity), attachedToEntity(true), useTime(false) {}
	// Returns true if entity with this component should be despawned
	inline bool update(const entt::DefaultRegistry& registry, float deltaTime) {
		bool despawn = false;
		if (attachedToEntity) {
			if (!registry.valid(attachedTo)) despawn = true;
		}
		if (useTime) {
			time += deltaTime;
			if (time >= maxTime) despawn = true;
		}
		return despawn;
	}

	inline void setMaxTime(float maxTime) {
		useTime = true;
		this->maxTime = maxTime;
	}

private:
	// Time since the entity with this component spawned
	float time = 0;
	float maxTime;
	bool useTime;

	bool attachedToEntity = false;
	uint32_t attachedTo;
};

// Component assigned only to a single entity - the level manager
class LevelManagerTag {
public:
	inline LevelManagerTag(std::shared_ptr<Level> level) : level(level) {}
	void update(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, float deltaTime);

	inline float getTimeSinceStartOfLevel() { return timeSinceStartOfLevel; }
	inline float getTimeSinceLastEnemySpawn() { return timeSinceLastEnemySpawn; }

	inline void setTimeSinceLastEnemySpawn(float timeSinceLastEnemySpawn) { this->timeSinceLastEnemySpawn = timeSinceLastEnemySpawn; }

private:
	// Time since the start of the level
	float timeSinceStartOfLevel = 0;
	// Time since the last enemy spawn
	// If no enemies have been spawned yet, this is the same as timeSinceStartOfLevel
	float timeSinceLastEnemySpawn = 0;

	std::shared_ptr<Level> level;
	// Current index in list of groups of enemies to be spawned from the level
	int currentEnemyGroupSpawnIndex = -1;
};

class EnemyBulletComponent {
public:
	EnemyBulletComponent(int attackID, int attackPatternID, int enemyID, int enemyPhaseID) : attackID(attackID), attackPatternID(attackPatternID), enemyID(enemyID), enemyPhaseID(enemyPhaseID) {}

private:
	// The attack this bullet belongs to
	int attackID;
	// The attack pattern this bullet belongs to
	int attackPatternID;
	// The enemy this bullet belongs to
	int enemyID;
	// The enemy phase this bullet belongs to
	int enemyPhaseID;
};

class PlayerBulletComponent {
public:
	PlayerBulletComponent(int attackID, int attackPatternID) : attackID(attackID), attackPatternID(attackPatternID) {}

private:
	// The attack this bullet belongs to
	int attackID;
	// The attack pattern this bullet belongs to
	int attackPatternID;
};

/*
Component for entities that act only as a reference point for a single MovablePoint.
*/
class SimpleEMPReferenceComponent {

};

/*
Component for entities that will spawn enemy/player bullets after some time.
*/
class EMPSpawnerComponent {
public:
	/*
	Constructor for an enemy bullets spawner.

	emps - the EMPs that will be spawned by this entity and, if applicable, will be spawned with respect to parent; must be sorted ascending by time of spawn
	parent - the entity that the spawned EMPs will be spawned in reference to; may be unused depending on the EMP's spawn type
	attackID - the ID of the attack each EMP originated from
	attackPattern - same but attack pattern
	enemyID - same but enemy
	enemyPhaseID - same but enemy phase
	*/
	EMPSpawnerComponent(std::vector<std::shared_ptr<EditorMovablePoint>> emps, uint32_t parent, int attackID, int attackPatternID, int enemyID, int enemyPhaseID);
	/*
	Constructor for an player bullets spawner.

	emps - the EMPs that will be spawned by this entity and, if applicable, will be spawned with respect to parent; must be sorted ascending by time of spawn
	parent - the entity that the spawned EMPs will be spawned in reference to; may be unused depending on the EMP's spawn type
	attackID - the ID of the attack each EMP originated from
	attackPattern - same but attack pattern
	*/
	EMPSpawnerComponent(std::vector<std::shared_ptr<EditorMovablePoint>> emps, uint32_t parent, int attackID, int attackPatternID);

	void EMPSpawnerComponent::update(entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, EntityCreationQueue& queue, float deltaTime);

private:
	bool isEnemyBulletSpawner;

	std::queue<std::shared_ptr<EditorMovablePoint>> emps;
	uint32_t parent;
	int attackID;
	int attackPatternID;
	int enemyID;
	int enemyPhaseID;

	// Time since this entity was spawned
	float time = 0;
};
