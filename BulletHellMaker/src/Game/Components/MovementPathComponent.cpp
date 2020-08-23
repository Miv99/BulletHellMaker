#include <Game/Components/MovementPathComponent.h>

#include <Game/Components/PositionComponent.h>
#include <Game/EntityCreationQueue.h>
#include <LevelPack/EditorMovablePoint.h>
#include <LevelPack/EditorMovablePointAction.h>
#include <DataStructs/MovablePoint.h>

MovementPathComponent::MovementPathComponent(EntityCreationQueue& queue, uint32_t self, entt::DefaultRegistry& registry, uint32_t entity, 
	std::shared_ptr<EMPSpawnType> spawnType, std::vector<std::shared_ptr<EMPAction>> actions, float initialTime) 
	: actions(actions), time(initialTime) {
	initialSpawn(registry, entity, spawnType, actions);
	// Can call update with deltaTime of 0 because time was initialized to initialTime already
	update(queue, registry, self, registry.get<PositionComponent>(self), 0);
}

MovementPathComponent::MovementPathComponent(EntityCreationQueue& queue, uint32_t self, entt::DefaultRegistry& registry, uint32_t entity, 
	MPSpawnInformation spawnInfo, std::vector<std::shared_ptr<EMPAction>> actions, float initialTime) 
	: actions(actions), time(initialTime) {
	initialSpawn(registry, entity, spawnInfo, actions);
	// Can call update with deltaTime of 0 because time was initialized to initialTime already
	update(queue, registry, self, registry.get<PositionComponent>(self), 0);
}

void MovementPathComponent::update(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, PositionComponent& entityPosition, float deltaTime) {
	time += deltaTime;
	sf::Vector2f tempReference(0, 0);
	// While loop for actions with lifespan of 0 like DetachFromParent 
	while (currentActionsIndex < actions.size() && time >= path->getLifespan()) {
		// Set this component's entity's position to last point of the ending MovablePoint to prevent inaccuracies from building up in updates
		if (useReferenceEntity) {
			auto& pos = registry.get<PositionComponent>(referenceEntity);
			entityPosition.setPosition(path->compute(sf::Vector2f(pos.getX(), pos.getY()), path->getLifespan()));
		} else {
			entityPosition.setPosition(path->compute(tempReference, path->getLifespan()));
		}

		time -= path->getLifespan();
		previousPaths.push_back(path);
		path = actions[currentActionsIndex]->execute(queue, registry, entity, time);
		currentActionsIndex++;

		tempReference.x = entityPosition.getX();
		tempReference.y = entityPosition.getY();
	}
	if (time <= path->getLifespan()) {
		if (useReferenceEntity) {
			auto& pos = registry.get<PositionComponent>(referenceEntity);
			entityPosition.setPosition(path->compute(sf::Vector2f(pos.getX(), pos.getY()), time));
		} else {
			entityPosition.setPosition(path->compute(tempReference, time));
		}
	} else {
		// The path's lifespan has been exceeded and there are no more paths to execute, so just stay at the last position on the path, relative to the reference entity

		if (useReferenceEntity) {
			auto& pos = registry.get<PositionComponent>(referenceEntity);
			entityPosition.setPosition(path->compute(sf::Vector2f(pos.getX(), pos.getY()), path->getLifespan()));
		} else {
			entityPosition.setPosition(path->compute(tempReference, path->getLifespan()));
		}
	}
}

sf::Vector2f MovementPathComponent::getPreviousPosition(entt::DefaultRegistry& registry, float secondsAgo) const {
	// This function assumes that if a reference entity has no MovementPathComponent, it has stayed in the same position its entire lifespan

	float curTime = time - secondsAgo;
	// Account for float inaccuracies
	if (curTime >= -0.0001f) {
		// this component's entity was on the same path [secondsAgo] seconds ago

		// curTime was probably supposed to be 0 but ended up being < 0 from float inaccuracies, so set it to 0
		if (curTime < 0) {
			curTime = 0;
		}

		if (useReferenceEntity) {
			if (registry.has<MovementPathComponent>(referenceEntity)) {
				auto pos = registry.get<MovementPathComponent>(referenceEntity).getPreviousPosition(registry, secondsAgo);
				return path->compute(sf::Vector2f(pos.x, pos.y), curTime);
			} else {
				auto& pos = registry.get<PositionComponent>(referenceEntity);
				return path->compute(sf::Vector2f(pos.getX(), pos.getY()), curTime);
			}
		} else {
			return path->compute(sf::Vector2f(0, 0), curTime);
		}
	} else {
		int curPathIndex = previousPaths.size();
		while (curTime < 0 && curPathIndex - 1 >= 0) {
			assert(curPathIndex - 1 >= 0 && "Somehow looking back in the past too far");
			curTime += previousPaths[curPathIndex - 1]->getLifespan();
			curPathIndex--;
		}

		if (useReferenceEntity) {
			if (registry.has<MovementPathComponent>(referenceEntity)) {
				auto pos = registry.get<MovementPathComponent>(referenceEntity).getPreviousPosition(registry, secondsAgo);
				return previousPaths[curPathIndex]->compute(sf::Vector2f(pos.x, pos.y), curTime);
			} else {
				auto& pos = registry.get<PositionComponent>(referenceEntity);
				return previousPaths[curPathIndex]->compute(sf::Vector2f(pos.getX(), pos.getY()), curTime);
			}
		} else {
			return previousPaths[curPathIndex]->compute(sf::Vector2f(0, 0), curTime);
		}
	}
}

void MovementPathComponent::setPath(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, PositionComponent& entityPosition, std::shared_ptr<MovablePoint> newPath, float timeLag) {
	// Put old path into history

	// Since the old path ended unexpectedly, change its lifespan
	path->setLifespan(time - timeLag);
	previousPaths.push_back(path);
	path = newPath;

	time = timeLag;
	update(queue, registry, entity, entityPosition, 0);
}

void MovementPathComponent::initialSpawn(entt::DefaultRegistry& registry, uint32_t entity, std::shared_ptr<EMPSpawnType> spawnType, std::vector<std::shared_ptr<EMPAction>>& actions) {
	auto spawnInfo = spawnType->getSpawnInfo(registry, entity, time);
	useReferenceEntity = spawnInfo.useReferenceEntity;
	referenceEntity = spawnInfo.referenceEntity;

	// Set a temporary path until the first update() or setPath() call
	path = std::make_shared<StationaryMP>(spawnInfo.position, 0);
}

void MovementPathComponent::initialSpawn(entt::DefaultRegistry& registry, uint32_t entity, MPSpawnInformation spawnInfo, std::vector<std::shared_ptr<EMPAction>>& actions) {
	useReferenceEntity = spawnInfo.useReferenceEntity;
	referenceEntity = spawnInfo.referenceEntity;

	// Set a temporary path until the first update() or setPath() call
	path = std::make_shared<StationaryMP>(spawnInfo.position, 0);
}

void MovementPathComponent::setReferenceEntity(uint32_t reference) {
	useReferenceEntity = true;
	referenceEntity = reference;
}

bool MovementPathComponent::usesReferenceEntity() const { 
	return useReferenceEntity;
}

uint32_t MovementPathComponent::getReferenceEntity() const { 
	return referenceEntity;
}

std::shared_ptr<MovablePoint> MovementPathComponent::getPath() const {
	return path;
}

float MovementPathComponent::getTime() const { 
	return time;
}