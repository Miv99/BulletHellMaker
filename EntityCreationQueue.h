#pragma once
#include <entt/entt.hpp>
#include <memory>
#include <queue>
#include "Components.h"

class EditorMovablePoint;
class SpriteLoader;
class EditorEnemy;

/*
A command that spawns entity/entities. The amount spawned is always known.
*/
class EntityCreationCommand {
public:
	inline EntityCreationCommand(entt::DefaultRegistry& registry) : registry(registry) {}

	virtual void execute(EntityCreationQueue& queue) = 0;
	virtual int getEntitiesQueuedCount() = 0;

protected:
	entt::DefaultRegistry& registry;
};

/*
Command for creating an entity that acts as a part of a shadow trail.
*/
class SpawnShadowTrailCommand : public EntityCreationCommand {
public:
	inline SpawnShadowTrailCommand(entt::DefaultRegistry& registry, sf::Sprite sprite, float x, float y, float shadowLifespan) : EntityCreationCommand(registry), sprite(sprite), x(x), y(y), shadowLifespan(shadowLifespan) {}

	void execute(EntityCreationQueue& queue) override;
	int getEntitiesQueuedCount() override;

private:
	sf::Sprite sprite;
	// Global positions
	float x;
	float y;
	// The lifespan of each shadow, in seconds
	float shadowLifespan;
};

/*
Command for creating an enemy.
*/
class SpawnEnemyCommand : public EntityCreationCommand {
public:
	SpawnEnemyCommand(entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, std::shared_ptr<EditorEnemy> enemyInfo, float x, float y);

	void execute(EntityCreationQueue& queue) override;
	int getEntitiesQueuedCount() override;

private:
	SpriteLoader& spriteLoader;
	std::shared_ptr<EditorEnemy> enemyInfo;
	float x;
	float y;
};

/*
Command for creating the reference entity for a DetachFromParentEMPA and then setting it to
the executor's reference entity.
*/
class EMPADetachFromParentCommand : public EntityCreationCommand {
public:
	inline EMPADetachFromParentCommand(entt::DefaultRegistry& registry, uint32_t entity, float lastPosX, float lastPosY) : EntityCreationCommand(registry), entity(entity), lastPosX(lastPosX), lastPosY(lastPosY) {}

	void execute(EntityCreationQueue& queue) override;
	int getEntitiesQueuedCount() override;

private:
	uint32_t entity;
	float lastPosX;
	float lastPosY;
};

/*
Command for creating the reference entity at the last position of some entity and then setting it as
the executor's reference entity.
*/
class CreateMovementRefereceEntityCommand : public EntityCreationCommand {
public:
	inline CreateMovementRefereceEntityCommand(entt::DefaultRegistry& registry, uint32_t entity, float timeLag, float lastPosX, float lastPosY) : EntityCreationCommand(registry), entity(entity), timeLag(timeLag), lastPosX(lastPosX), lastPosY(lastPosY) {}

	void execute(EntityCreationQueue& queue) override;
	int getEntitiesQueuedCount() override;

private:
	uint32_t entity;
	float timeLag;
	float lastPosX;
	float lastPosY;
};

/*
Command for creating the entity/entities associated with an attack by an enemy.
*/
class EMPSpawnFromEnemyCommand : public EntityCreationCommand {
public:
	EMPSpawnFromEnemyCommand(entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, std::shared_ptr<EditorMovablePoint> emp, uint32_t entity, float timeLag, int attackID, int attackPatternID, int enemyID, int enemyPhaseID, bool playAttackAnimation);

	void execute(EntityCreationQueue& queue) override;
	int getEntitiesQueuedCount() override;

private:
	SpriteLoader& spriteLoader;
	std::shared_ptr<EditorMovablePoint> emp;
	// The enemy executing the attack
	uint32_t entity;
	float timeLag;
	int attackID;
	int attackPatternID;
	int enemyID;
	int enemyPhaseID;
	bool playAttackAnimation;
};

/*
Command for creating the entity/entities associated with an attack by a player.
*/
class EMPSpawnFromPlayerCommand : public EntityCreationCommand {
public:
	EMPSpawnFromPlayerCommand(entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, std::shared_ptr<EditorMovablePoint> emp, uint32_t entity, float timeLag, int attackID, int attackPatternID, bool playAttackAnimation);

	void execute(EntityCreationQueue& queue) override;
	int getEntitiesQueuedCount() override;

private:
	SpriteLoader& spriteLoader;
	std::shared_ptr<EditorMovablePoint> emp;
	// The player executing the attack
	uint32_t entity;
	float timeLag;
	int attackID;
	int attackPatternID;
	bool playAttackAnimation;
};

/*
A queue that holds commands that create entities.
The purpose of this class is to allow the queueing of entity creations so that sufficient space can be
reserved in the registry before the entities are created, since space cannot be reserved while the registry
is being looped through.
*/
class EntityCreationQueue {
public:
	inline EntityCreationQueue(entt::DefaultRegistry& registry) : registry(registry) {}

	inline void addCommand(std::unique_ptr<EntityCreationCommand> command) {
		queue.push(std::move(command));
	}
	inline void executeAll() {
		while (!queue.empty()) {
			std::unique_ptr<EntityCreationCommand> command = std::move(queue.front());
			queue.pop();

			// Reserve space for the entities that will be spawned by the command
			int reserve = registry.alive() + command->getEntitiesQueuedCount();
			//std::cout << reserve << std::endl;
			registry.reserve(reserve);
			registry.reserve<DespawnComponent>(reserve);
			registry.reserve<EnemyBulletComponent>(reserve);
			registry.reserve<EnemyComponent>(reserve);
			registry.reserve<HealthComponent>(reserve);
			registry.reserve<HitboxComponent>(reserve);
			registry.reserve<MovementPathComponent>(reserve);
			registry.reserve<PlayerBulletComponent>(reserve);
			registry.reserve<PositionComponent>(reserve);
			registry.reserve<SpriteComponent>(reserve);
			registry.reserve<SimpleEMPReferenceComponent>(reserve);
			registry.reserve<EMPSpawnerComponent>(reserve);
			// Ignore level manager component since there can only be one

			command->execute(*this);
		}
	}

private:
	entt::DefaultRegistry& registry;
	std::queue<std::unique_ptr<EntityCreationCommand>> queue;
};