#include <Game/Components/CollectibleComponent.h>

#include <LevelPack/Item.h>
#include <Game/EntityCreationQueue.h>
#include <Game/Components/MovementPathComponent.h>
#include <Game/Components/DespawnComponent.h>
#include <Game/Components/PositionComponent.h>
#include <Game/Components/HitboxComponent.h>
#include <Game/Components/PlayerTag.h>

CollectibleComponent::CollectibleComponent(std::shared_ptr<Item> item, float activationRadius) 
	: item(item), activationRadius(activationRadius) {
}

void CollectibleComponent::activate(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t self) {
	// Item is activated, so begin moving towards the player

	auto speed = std::make_shared<PiecewiseTFV>();
	// Speed starts off quickly at 450 and slows to 350 by t=2
	speed->insertSegment(std::make_pair(0, std::make_shared<DampenedEndTFV>(450.0f, 350.0f, 2.0f, 2)));
	// Speed then maintains a constant 350 forever
	speed->insertSegment(std::make_pair(2.0f, std::make_shared<ConstantTFV>(350.0f)));
	auto newPath = std::make_shared<HomingMP>(std::numeric_limits<float>::max(), speed, std::make_shared<ConstantTFV>(0.12f), self, registry.attachee<PlayerTag>(), registry);
	registry.get<MovementPathComponent>(self).setPath(queue, registry, self, registry.get<PositionComponent>(self), newPath, 0);
	// Make sure item can't despawn
	registry.get<DespawnComponent>(self).setMaxTime(std::numeric_limits<float>::max());
	activated = true;
}

std::shared_ptr<Item> CollectibleComponent::getItem() {
	return item;
}