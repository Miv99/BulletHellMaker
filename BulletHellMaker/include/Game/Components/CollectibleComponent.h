#pragma once
#include <entt/entt.hpp>

class Item;
class EntityCreationQueue;

/*
Component for items that can be collected by the player.
Entities with this component must also have HitboxComponent, PositionComponent, and MovementPathComponent (which does not need to have a path).
*/
class CollectibleComponent {
public:
	CollectibleComponent(std::shared_ptr<Item> item, float activationRadius);

	void activate(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t self);
	std::shared_ptr<Item> getItem();
	inline bool isActivated() { return activated; }

private:
	bool activated = false;
	std::shared_ptr<Item> item;
	float activationRadius;
};