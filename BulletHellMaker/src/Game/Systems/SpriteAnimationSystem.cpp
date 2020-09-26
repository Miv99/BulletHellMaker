#include <Game/Systems/SpriteAnimationSystem.h>

#include <SFML/Graphics.hpp>

#include <Game/Components/PositionComponent.h>
#include <Game/Components/AnimatableSetComponent.h>
#include <Game/Components/SpriteComponent.h>

void SpriteAnimationSystem::update(float deltaTime) {
	auto view = registry.view<SpriteComponent>();
	auto animatableSetView = registry.view<PositionComponent, AnimatableSetComponent, SpriteComponent>(entt::persistent_t{});

	animatableSetView.each([this, deltaTime](auto entity, auto& position, auto& set, auto& sprite) {
		set.update(spriteLoader, position.getX(), position.getY(), sprite, deltaTime);
	});

	view.each([this, deltaTime](auto entity, auto& sprite) {
		sprite.update(deltaTime);
	});

	animatableSetView.each([this](auto entity, auto& position, auto& set, auto& sprite) {
		// Update again with 0 delta time in case some state needs to be changed
		set.update(spriteLoader, position.getX(), position.getY(), sprite, 0);
	});
}
