#include "PlayerSystem.h"
#include "Constants.h"

void PlayerSystem::update(float deltaTime) {
	auto& playerTag = registry.get<PlayerTag>();

	int verticalInput = 0;
	int horizontalInput = 0;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
		verticalInput++;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
		verticalInput--;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
		horizontalInput--;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
		horizontalInput++;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
		playerTag.setFocused(true);
	} else {
		playerTag.setFocused(false);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
		playerTag.setAttacking(true);
	} else {
		playerTag.setAttacking(false);
	}

	uint32_t playerEntity = registry.attachee<PlayerTag>();

	playerTag.update(deltaTime, levelPack, queue, spriteLoader, registry, playerEntity);

	// Movement
	float speed = playerTag.getFocused() ? playerTag.getFocusedSpeed() : playerTag.getSpeed();
	auto& pos = registry.get<PositionComponent>(playerEntity);
	float prevX = pos.getX();
	float prevY = pos.getY();
	pos.setX(pos.getX() + horizontalInput * speed * deltaTime);
	pos.setY(pos.getY() + verticalInput * speed * deltaTime);
	// Calculate angle of movement
	auto& hitbox = registry.get<HitboxComponent>(playerEntity);
	if (horizontalInput != 0 || verticalInput != 0) {
		float angle = std::atan2(pos.getY() - prevY, pos.getX() - prevX);

		// Rotate sprite and hitbox
		auto& sprite = registry.get<SpriteComponent>(playerEntity);
		sprite.rotate(angle);
		if (sprite.getSprite()) {
			// Rotate hitbox according to sprite orientation
			hitbox.rotate(sprite.getSprite());
		}
	}
	
	// Make sure player doesn't go out of bounds
	if (pos.getX() + hitbox.getX() < 0) {
		pos.setX(hitbox.getX());
	} else if (pos.getX() + hitbox.getX() > MAP_WIDTH) {
		pos.setX(MAP_WIDTH);
	}
	if (pos.getY() + hitbox.getY() < 0) {
		pos.setY(hitbox.getY());
	} else if (pos.getY() + hitbox.getY() > MAP_HEIGHT) {
		pos.setY(MAP_HEIGHT);
	}
}

void PlayerSystem::handleEvent(sf::Event event) {
	if (event.type == sf::Event::KeyPressed) {
		if (event.key.code == sf::Keyboard::X) {
			registry.get<PlayerTag>().activateBomb();
		}
	}
}

void PlayerSystem::onResume() {
	
}
