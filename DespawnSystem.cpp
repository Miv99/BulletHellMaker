#include "DespawnSystem.h"
#include <set>

void dfsInsertChildren(entt::DefaultRegistry& registry, std::set<uint32_t>& dest, const std::vector<uint32_t>& source) {
	for (uint32_t entity : source) {
		if (registry.valid(entity)) {
			dfsInsertChildren(registry, dest, registry.get<DespawnComponent>(entity).getChildren());
			dest.insert(entity);
		}
	}
}

void DespawnSystem::update(float deltaTime) {
	auto view = registry.view<DespawnComponent>();
	std::set<uint32_t> deletionQueue;

	view.each([&](auto entity, auto& despawn) {
		if (despawn.update(registry, deltaTime)) {
			dfsInsertChildren(registry, deletionQueue, despawn.getChildren());

			despawn.onDespawn(entity);
			registry.destroy(entity);
		}
	});

	for (uint32_t entity : deletionQueue) {
		if (registry.valid(entity)) {
			if (registry.has<DespawnComponent>(entity)) {
				auto& despawn = registry.get<DespawnComponent>(entity);
				// Remove attachment in case its parent is supposed to despawn when all its children despawn
				despawn.removeEntityAttachment(registry, entity);
				despawn.onDespawn(entity);
			}
			registry.destroy(entity);
		}
	}
}
