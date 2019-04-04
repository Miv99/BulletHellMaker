#include "DespawnSystem.h"
#include <set>

void dfsInsertChildren(entt::DefaultRegistry& registry, std::set<uint32_t>& dest, const std::vector<uint32_t>& source) {
	for (uint32_t entity : source) {
		dfsInsertChildren(registry, dest, registry.get<DespawnComponent>(entity).getChildren());
		dest.insert(entity);
	}
}

void DespawnSystem::update(float deltaTime) {
	auto view = registry.view<DespawnComponent>();
	std::set<uint32_t> deletionQueue;

	view.each([&](auto entity, auto& despawn) {
		if (despawn.update(registry, deltaTime)) {
			dfsInsertChildren(registry, deletionQueue, despawn.getChildren());
			registry.destroy(entity);
		}
	});

	for (uint32_t entity : deletionQueue) {
		if (registry.valid(entity)) {
			registry.destroy(entity);
		}
	}
}
