#include "EnemySystem.h"

void EnemySystem::update(float deltaTime) {
	auto view = registry.view<EnemyComponent>();

	view.each([&](auto entity, auto& enemy) {
		enemy.update(queue, spriteLoader, levelPack, registry, entity, deltaTime);
	});
}