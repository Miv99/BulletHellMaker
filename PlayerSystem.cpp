#include "PlayerSystem.h"
#include "Constants.h"

void PlayerSystem::update(float deltaTime) {
	uint32_t playerEntity = registry.attachee<PlayerTag>();
	auto& playerTag = registry.get<PlayerTag>();
	float speed = focused ? playerTag.getFocusedSpeed() : playerTag.getSpeed();

	// Movement
	auto& pos = registry.get<PositionComponent>(playerEntity);
	pos.setX(pos.getX() + horizontalInput * speed * deltaTime);
	pos.setY(pos.getY() + verticalInput * speed * deltaTime);
	// Make sure player doesn't go out of bounds
	auto& hitbox = registry.get<HitboxComponent>(playerEntity);
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
