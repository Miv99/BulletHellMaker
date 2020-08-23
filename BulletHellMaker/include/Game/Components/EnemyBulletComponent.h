#pragma once
#include <map>

#include <entt/entt.hpp>

enum class BULLET_ON_COLLISION_ACTION;

/*
Component for a bullet that originated from an enemy.
*/
class EnemyBulletComponent {
public:
	EnemyBulletComponent(int attackID, int attackPatternID, int enemyID, int enemyPhaseID, int damage, 
		BULLET_ON_COLLISION_ACTION onCollisionAction, float pierceResetTime);

	void update(float deltaTime);

	/*
	Should be called whenever this component's entity collides with another entity 
	and the other entity does not die.
	*/
	void onCollision(uint32_t collidedWith);

	/*
	Returns whether a collision between this component's entity and another
	entity, if it is possible, should count as a valid collision.
	*/
	bool isValidCollision(uint32_t collidingWith);

	int getDamage() const;
	BULLET_ON_COLLISION_ACTION getOnCollisionAction() const;
	float getPierceResetTime() const;

private:
	// The attack this bullet belongs to
	int attackID;
	// The attack pattern this bullet belongs to
	int attackPatternID;
	// The enemy this bullet belongs to
	int enemyID;
	// The enemy phase this bullet belongs to
	int enemyPhaseID;

	int damage;
	BULLET_ON_COLLISION_ACTION onCollisionAction;
	// Used only when onCollisionAction is PIERCE_ENTITY.
	// Map of entity to time remaining until that entity is able to be hit by this same bullet again.
	// If an entity in the map dies, it is not removed from the map because the chances of the same 
	// entity id being spawned to another hittable entity is so small that it's not worth the computational time.
	std::map<uint32_t, float> ignoredEntities;
	// Time after hitting an enemy that the entity is able to be hit by this same bullet again; only for PIERCE_ENTITY onCollisionAction
	float pierceResetTime;
};