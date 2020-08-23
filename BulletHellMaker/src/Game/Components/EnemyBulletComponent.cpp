#include <Game/Components/EnemyBulletComponent.h>

#include <Game/Systems/CollisionSystem.h>

EnemyBulletComponent::EnemyBulletComponent(int attackID, int attackPatternID, int enemyID, int enemyPhaseID, int damage, 
	BULLET_ON_COLLISION_ACTION onCollisionAction, float pierceResetTime) 
	: attackID(attackID), attackPatternID(attackPatternID), enemyID(enemyID), enemyPhaseID(enemyPhaseID), damage(damage), 
	onCollisionAction(onCollisionAction), pierceResetTime(pierceResetTime) {
}

void EnemyBulletComponent::update(float deltaTime) {
	for (auto& it = ignoredEntities.begin(); it != ignoredEntities.end(); it++) {
		it->second -= deltaTime;
	}
}

void EnemyBulletComponent::onCollision(uint32_t collidedWith) {
	if (onCollisionAction != BULLET_ON_COLLISION_ACTION::PIERCE_ENTITY) return;

	if (ignoredEntities.find(collidedWith) == ignoredEntities.end()) {
		ignoredEntities.insert(std::make_pair(collidedWith, pierceResetTime));
	} else {
		ignoredEntities[collidedWith] = pierceResetTime;
	}
}

bool EnemyBulletComponent::isValidCollision(uint32_t collidingWith) {
	if (onCollisionAction != BULLET_ON_COLLISION_ACTION::PIERCE_ENTITY) return true;
	return ignoredEntities.find(collidingWith) == ignoredEntities.end() || ignoredEntities[collidingWith] <= 0;
}

int EnemyBulletComponent::getDamage() const { 
	return damage;
}

BULLET_ON_COLLISION_ACTION EnemyBulletComponent::getOnCollisionAction() const {
	return onCollisionAction;
}

float EnemyBulletComponent::getPierceResetTime() const { 
	return pierceResetTime;
}