#include "PlayerSystem.h"
#include "Constants.h"

void PlayerSystem::update(float deltaTime) {
	uint32_t playerEntity = registry.attachee<PlayerTag>();
	auto& playerTag = registry.get<PlayerTag>();
	if (playerTag.justIncreasedPowerTier()) {
		timeSinceNewAttackPattern = 0;
		currentAttackIndex = -1;
	}
	float speed = focused ? playerTag.getFocusedSpeed() : playerTag.getSpeed();

	// Movement
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

	// Attack
	if (attacking) {
		auto currentAttackPattern = focused ? playerTag.getFocusedAttackPattern() : playerTag.getAttackPattern();
		float attackPatternTotalTime = focused ? playerTag.getFocusedAttackPatternTotalTime() : playerTag.getAttackPatternTotalTime();
		timeSinceNewAttackPattern += deltaTime;

		while (currentAttackIndex + 1 < currentAttackPattern->getAttacksCount()) {
			auto nextAttack = currentAttackPattern->getAttackData(currentAttackIndex + 1);
			if (timeSinceNewAttackPattern >= nextAttack.first) {
				currentAttackIndex++;
				levelPack.getAttack(nextAttack.second)->executeAsPlayer(queue, spriteLoader, registry, playerEntity, timeSinceNewAttackPattern - nextAttack.first, currentAttackPattern->getID());
			} else {
				break;
			}
		}
		if (currentAttackIndex + 1 == currentAttackPattern->getAttacksCount()) {
			while (timeSinceNewAttackPattern >= attackPatternTotalTime) {
				timeSinceNewAttackPattern -= attackPatternTotalTime;
				currentAttackIndex = -1;
			}
		}
	}
}

void PlayerSystem::handleEvent(sf::Event event) {
	if (event.type == sf::Event::KeyPressed) {
		if (event.key.code == sf::Keyboard::Up) {
			verticalInput++;
		} else if (event.key.code == sf::Keyboard::Down) {
			verticalInput--;
		} else if (event.key.code == sf::Keyboard::Left) {
			horizontalInput--;
		} else if (event.key.code == sf::Keyboard::Right) {
			horizontalInput++;
		} else if (event.key.code == sf::Keyboard::LShift) {
			focused = true;
			timeSinceNewAttackPattern = 0;
			currentAttackIndex = -1;
		} else if (event.key.code == sf::Keyboard::Z) {
			attacking = true;
			timeSinceNewAttackPattern = 0;
			currentAttackIndex = -1;
		}
	} else if (event.type == sf::Event::KeyReleased) {
		if (event.key.code == sf::Keyboard::Up) {
			verticalInput--;
		} else if (event.key.code == sf::Keyboard::Down) {
			verticalInput++;
		} else if (event.key.code == sf::Keyboard::Left) {
			horizontalInput++;
		} else if (event.key.code == sf::Keyboard::Right) {
			horizontalInput--;
		} else if (event.key.code == sf::Keyboard::LShift) {
			focused = false;
			timeSinceNewAttackPattern = 0;
			currentAttackIndex = -1;
		} else if (event.key.code == sf::Keyboard::Z) {
			attacking = false;
			timeSinceNewAttackPattern = 0;
			currentAttackIndex = -1;
		}
	}
}

void PlayerSystem::onResume() {
	verticalInput = 0;
	horizontalInput = 0;
	focused = false;
	attacking = false;

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
		focused = true;
	} 
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
		attacking = true;
	}
}
