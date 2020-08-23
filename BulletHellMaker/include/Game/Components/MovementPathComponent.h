#pragma once
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

class MovablePoint;
struct MPSpawnInformation;
class EntityCreationQueue;
class EMPSpawnType;
class EMPAction;
class PositionComponent;

/*
Component for an entity that follows a movement path modelled by a MovablePoint.
*/
class MovementPathComponent {
public:
	/*
	entity - the entity that this component's entity should be attached to, if any (determined by spawnType)
	spawnType - how the this component's entity is initially being spawned
	actions - a list of actions that determines this component's entity's movement path
	initialTime - how long ago this component's entity should have been spawned
	*/
	MovementPathComponent(EntityCreationQueue& queue, uint32_t self, entt::DefaultRegistry& registry, uint32_t entity, std::shared_ptr<EMPSpawnType> spawnType, std::vector<std::shared_ptr<EMPAction>> actions, float initialTime);
	/*
	entity - the entity that this component's entity should be attached to, if any (determined by spawnInfo)
	spawnInfo - how the this component's entity is initially being spawned
	actions - a list of actions that determines this component's entity's movement path
	initialTime - how long ago this component's entity should have been spawned
	*/
	MovementPathComponent(EntityCreationQueue& queue, uint32_t self, entt::DefaultRegistry& registry, uint32_t entity, MPSpawnInformation spawnInfo, std::vector<std::shared_ptr<EMPAction>> actions, float initialTime);

	/*
	Updates elapsed time and updates the entity's position along its path.
	*/
	void update(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, PositionComponent& entityPosition, float deltaTime);

	/*
	Returns this component's entity's position some time ago.
	*/
	sf::Vector2f getPreviousPosition(entt::DefaultRegistry& registry, float secondsAgo) const;

	bool usesReferenceEntity() const;
	uint32_t getReferenceEntity() const;
	std::shared_ptr<MovablePoint> getPath() const;
	float getTime() const;

	/*
	Sets the reference entity of this component's entity.
	The PositionComponent of this component's entity should be updated after this call.
	*/
	void setReferenceEntity(uint32_t reference);
	/*
	Changes the path of an entity.

	timeLag - number of seconds ago that the path change should have happened
	*/
	void setPath(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, PositionComponent& entityPosition, std::shared_ptr<MovablePoint> newPath, float timeLag);

private:
	bool useReferenceEntity;
	uint32_t referenceEntity;
	// Elapsed time since the the last path change
	float time;
	std::shared_ptr<MovablePoint> path;
	// Sorted descending in age (index 0 is the oldest path).
	std::vector<std::shared_ptr<MovablePoint>> previousPaths;
	// Actions to be carried out in order; each one changes pushes back an MP to path
	std::vector<std::shared_ptr<EMPAction>> actions;
	int currentActionsIndex = 0;

	void initialSpawn(entt::DefaultRegistry& registry, uint32_t entity, std::shared_ptr<EMPSpawnType> spawnType, std::vector<std::shared_ptr<EMPAction>>& actions);
	void initialSpawn(entt::DefaultRegistry& registry, uint32_t entity, MPSpawnInformation spawnInfo, std::vector<std::shared_ptr<EMPAction>>& actions);
};