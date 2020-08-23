#include <Game/Components/PlayerBulletComponent.h>

#include <Game/Systems/CollisionSystem.h>

PlayerBulletComponent::PlayerBulletComponent(int attackID, int attackPatternID, int damage, BULLET_ON_COLLISION_ACTION onCollisionAction, float pierceResetTime)
	: attackID(attackID), attackPatternID(attackPatternID), damage(damage), onCollisionAction(onCollisionAction), pierceResetTime(pierceResetTime) {
}

void PlayerBulletComponent::update(float deltaTime) {
	for (auto& it = ignoredEntities.begin(); it != ignoredEntities.end(); it++) {
		it->second -= deltaTime;
	}
}

void PlayerBulletComponent::onCollision(uint32_t collidedWith) {
	if (onCollisionAction != BULLET_ON_COLLISION_ACTION::PIERCE_ENTITY) return;

	if (ignoredEntities.find(collidedWith) == ignoredEntities.end()) {
		ignoredEntities.insert(std::make_pair(collidedWith, pierceResetTime));
	} else {
		ignoredEntities[collidedWith] = pierceResetTime;
	}
}

bool PlayerBulletComponent::isValidCollision(uint32_t collidingWith) {
	if (onCollisionAction != BULLET_ON_COLLISION_ACTION::PIERCE_ENTITY) return true;
	return ignoredEntities.find(collidingWith) == ignoredEntities.end() || ignoredEntities[collidingWith] <= 0;
}

int PlayerBulletComponent::getDamage() const {
	return damage;
}

BULLET_ON_COLLISION_ACTION PlayerBulletComponent::getOnCollisionAction() const {
	return onCollisionAction;
}

float PlayerBulletComponent::getPierceResetTime() const {
	return pierceResetTime;
}