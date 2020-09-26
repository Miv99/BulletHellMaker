#include <Game/Systems/EnemySystem.h>

#include <Game/Components/EnemyComponent.h>
#include <Game/EntityCreationQueue.h>

EnemySystem::EnemySystem(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry)
	: queue(queue), spriteLoader(spriteLoader), levelPack(levelPack), registry(registry) {
}

void EnemySystem::update(float deltaTime) {
	auto view = registry.view<EnemyComponent>();

	view.each([this, deltaTime](auto entity, auto& enemy) {
		enemy.update(queue, spriteLoader, levelPack, registry, entity, deltaTime);
	});
}