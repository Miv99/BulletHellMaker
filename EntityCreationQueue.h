#pragma once
#include <entt/entt.hpp>
#include <memory>
#include <deque>
#include "Components.h"
#include "EnemySpawn.h"
#include "Item.h"

class EditorMovablePoint;
class SpriteLoader;
class EditorEnemy;

static void reserveMemory(entt::DefaultRegistry& registry, int reserve) {
	// If capacity is reached, increase capacity by a set amount of entities
	if (reserve > registry.capacity()) {
		reserve = registry.capacity() + ENTITY_RESERVATION_INCREMENT;
	}

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
	registry.reserve<ShadowTrailComponent>(reserve);
	registry.reserve<AnimatableSetComponent>(reserve);
	registry.reserve<CollectibleComponent>(reserve);
	// Ignore level manager component since there can only be one
}

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
	inline SpawnShadowTrailCommand(entt::DefaultRegistry& registry, sf::Sprite sprite, float x, float y, float shadowRotationAngle, float shadowLifespan) : EntityCreationCommand(registry), sprite(sprite), x(x), y(y), angle(shadowRotationAngle), shadowLifespan(shadowLifespan) {}

	void execute(EntityCreationQueue& queue) override;
	int getEntitiesQueuedCount() override;

private:
	sf::Sprite sprite;
	// Global positions
	float x;
	float y;
	// The lifespan of each shadow, in seconds
	float shadowLifespan;
	// Angle of rotation of the shadow
	float angle;
};

/*
Command for creating an enemy.

Time lag cannot be accounted for with this command this because the condition for an enemy spawning is not always time-dependent.
*/
class SpawnEnemyCommand : public EntityCreationCommand {
public:
	SpawnEnemyCommand(entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, std::shared_ptr<EditorEnemy> enemyInfo, EnemySpawnInfo spawnInfo);

	void execute(EntityCreationQueue& queue) override;
	int getEntitiesQueuedCount() override;

private:
	SpriteLoader& spriteLoader;
	std::shared_ptr<EditorEnemy> enemyInfo;
	EnemySpawnInfo spawnInfo;
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
This command must be pushed to the front of the EntityCreationQueue.
*/
class CreateMovementReferenceEntityCommand : public EntityCreationCommand {
public:
	inline CreateMovementReferenceEntityCommand(entt::DefaultRegistry& registry, uint32_t entity, float timeLag, float lastPosX, float lastPosY) : EntityCreationCommand(registry), entity(entity), timeLag(timeLag), lastPosX(lastPosX), lastPosY(lastPosY) {}

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
	/*
	emp - the root of the EMP tree of the attack (see EditorAttack's mainEMP)
	entity - the enemy executing the attack
	timeLag - time when attack should have occurred minus time when the attack actually occurred
	attackPatternID - ID of enemy attack pattern that this attack came from
	enemyPhaseID - ID of enemy phase at the time of attack execution
	playAttackAnimation - whether the attack executor should play its attack animation
	*/
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
	/*
	emp - the root of the EMP tree of the attack (see EditorAttack's mainEMP)
	entity - the player executing the attack
	timeLag - time when attack should have occurred minus time when the attack actually occurred
	attackPatternID - ID of enemy attack pattern that this attack came from
	playAttackAnimation - whether the attack executor should play its attack animation
	*/
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
Command for creating a collectible health pack.
*/
class EMPDropItemCommand : public EntityCreationCommand {
public:
	/*
	entity - the entity that the item is dropping from
	x, y - the position the health pack is appearing from
	amount - the number of items dropping
	*/
	EMPDropItemCommand(entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, float x, float y, std::shared_ptr<Item> item, int amount);

	void execute(EntityCreationQueue& queue) override;
	int getEntitiesQueuedCount() override;

private:
	SpriteLoader& spriteLoader;
	float x;
	float y;
	int amount;
	std::shared_ptr<Item> item;
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

	inline void pushBack(std::unique_ptr<EntityCreationCommand> command) {
		queue.push_back(std::move(command));
	}
	inline void pushFront(std::unique_ptr<EntityCreationCommand> command) {
		queue.push_front(std::move(command));
	}
	inline void executeAll() {
		while (!queue.empty()) {
			std::unique_ptr<EntityCreationCommand> command = std::move(queue.front());
			queue.pop_front();

			// Reserve space for the entities that will be spawned by the command
			int reserve = registry.alive() + command->getEntitiesQueuedCount();
			reserveMemory(registry, reserve);

			command->execute(*this);
		}
	}

private:
	entt::DefaultRegistry& registry;
	std::deque<std::unique_ptr<EntityCreationCommand>> queue;
};