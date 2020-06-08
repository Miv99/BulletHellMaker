#include "CollectibleSystem.h"
#include <algorithm>

CollectibleSystem::CollectibleSystem(EntityCreationQueue & queue, entt::DefaultRegistry & registry, const LevelPack& levelPack, float mapWidth, float mapHeight) : queue(queue), registry(registry) {
	itemHitboxTable = SpatialHashTable<uint32_t>(mapWidth, mapHeight, levelPack.searchLargestItemCollectionHitbox() * 2.0f);
	activationTable = SpatialHashTable<uint32_t>(mapWidth, mapHeight, levelPack.searchLargestItemActivationHitbox() * 2.0f);
}

void CollectibleSystem::update(float deltaTime) {
	auto view = registry.view<PositionComponent, HitboxComponent, CollectibleComponent>();

	itemHitboxTable.clear();
	activationTable.clear();
	view.each([&](auto entity, auto& pos, auto& hitbox, auto& collectible) {
		if (collectible.isActivated()) {
			itemHitboxTable.insert(entity, hitbox, pos);
		} else {
			activationTable.insert(entity, hitbox.getX(), hitbox.getY(), collectible.getItem()->getActivationRadius(), pos);
		}
	});

	// Check if player is within activation radius of any collectibles
	if (registry.has<PlayerTag>()) {
		uint32_t player = registry.attachee<PlayerTag>();
		auto& playerHitbox = registry.get<HitboxComponent>(player);
		auto& playerPosition = registry.get<PositionComponent>(player);
		for (uint32_t entity : activationTable.getNearbyObjects(playerHitbox, playerPosition)) {
			auto& hitbox = registry.get<HitboxComponent>(entity);
			if (collides(playerPosition, playerHitbox, registry.get<PositionComponent>(entity), hitbox.getX(), hitbox.getY(), registry.get<CollectibleComponent>(entity).getItem()->getActivationRadius())) {
				registry.get<CollectibleComponent>(entity).activate(queue, registry, entity);
			}
		}
		// Check if player makes contact with any collectibles
		for (uint32_t entity : itemHitboxTable.getNearbyObjects(playerHitbox, playerPosition)) {
			if (!registry.get<DespawnComponent>(entity).isMarkedForDespawn() && collides(playerPosition, playerHitbox, registry.get<PositionComponent>(entity), registry.get<HitboxComponent>(entity))) {
				registry.get<CollectibleComponent>(entity).getItem()->onPlayerContact(registry, player);

				// Despawn the collectible
				if (registry.has<DespawnComponent>(entity)) {
					registry.get<DespawnComponent>(entity).setMaxTime(0);
				}
				else {
					registry.assign<DespawnComponent>(entity, 0);
				}
			}
		}
	}
}