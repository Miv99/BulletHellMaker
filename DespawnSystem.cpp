#include "DespawnSystem.h"

void DespawnSystem::update(float deltaTime) {
	auto view = registry.view<DespawnComponent>();

	view.each([&](auto entity, auto& despawn) {
		if (despawn.update(registry, deltaTime)) {
			registry.destroy(entity);
		}
	});
}
