#include "DebugRenderSystem.h"
#include "Components.h"

DebugRenderSystem::DebugRenderSystem(entt::DefaultRegistry& registry, sf::RenderWindow& window) : registry(registry), window(window) {
	circleFormat.setFillColor(sf::Color(sf::Color::Transparent));
	circleFormat.setOutlineThickness(3);
}

void DebugRenderSystem::update(float deltaTime) {
	auto view = registry.view<PositionComponent, HitboxComponent>(entt::persistent_t{});
	view.each([&](auto entity, auto position, auto hitbox) {
		// Radius takes into account outline thickness expanding outwards,
		// making the circle appear larger than it actually is
		circleFormat.setRadius(hitbox.getRadius() - circleFormat.getOutlineThickness());
		if (registry.attachee<PlayerTag>() == entity) {
			circleFormat.setOutlineColor(sf::Color(sf::Color::Blue));
		} else {
			circleFormat.setOutlineColor(sf::Color(sf::Color::Magenta));
		}
		circleFormat.setPosition(position.getX() + hitbox.getX() - circleFormat.getRadius(), -(position.getY() + hitbox.getY() + circleFormat.getRadius()));
		window.draw(circleFormat);
	});
}
