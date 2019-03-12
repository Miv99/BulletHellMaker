#include "Components.h"
#include "CollisionSystem.h"

float distance(const PositionComponent& p1, const PositionComponent& p2) {
	return sqrt(pow((p1.getX() - p2.getX()), 2) + pow((p1.getY() - p2.getY()), 2));
}

bool collides(const PositionComponent& p1, const HitboxComponent& h1, const PositionComponent& p2, const HitboxComponent& h2) {
	return distance(p1, p2) <= (h1.getRadius() + h2.getRadius());
}

float max(float a, float b) {
	return a > b ? a : b;
}

CollisionSystem::CollisionSystem(entt::DefaultRegistry & registry, float mapWidth, float mapHeight, const HitboxComponent& largestHitbox) : registry(registry) {
	defaultTableObjectMaxSize = 2.0 * max(mapWidth, mapHeight) / 10.0;
	defaultTable = SpatialHashTable<uint32_t>(mapWidth, mapHeight, defaultTableObjectMaxSize/2.0f);
	largeObjectsTable = SpatialHashTable<uint32_t>(mapWidth, mapHeight, largestHitbox.getRadius() * 2.0f);
}

void CollisionSystem::update(float deltaTime) {
	auto playerView = registry.view<PlayerTag, PositionComponent, HitboxComponent>(entt::persistent_t{});
	auto enemyView = registry.view<EnemyComponent, PositionComponent, HitboxComponent>(entt::persistent_t{});
	auto playerBulletView = registry.view<PlayerBulletComponent, PositionComponent, HitboxComponent>(entt::persistent_t{});
	auto enemyBulletView = registry.view<EnemyBulletComponent, PositionComponent, HitboxComponent>(entt::persistent_t{});

	// Reinsert all bullets, players, and enemies into tables
	defaultTable.clear();
	largeObjectsTable.clear();
	playerBulletView.each([&](auto entity, auto& playerBullet, auto& position, auto& hitbox) {
		// Check hitbox size for insertion into correct table
		if (hitbox.getRadius() < defaultTableObjectMaxSize) {
			defaultTable.insert(entity, hitbox, position);
		} else {
			largeObjectsTable.insert(entity, hitbox, position);
		}
	});
	enemyBulletView.each([&](auto entity, auto& enemyBullet, auto& position, auto& hitbox) {
		// Check hitbox size for insertion into correct table
		if (hitbox.getRadius() < defaultTableObjectMaxSize) {
			defaultTable.insert(entity, hitbox, position);
		} else {
			largeObjectsTable.insert(entity, hitbox, position);
		}
	});
	// largeObjectsTable always contains players and enemies
	playerView.each([&](auto entity, auto& player, auto& position, auto& hitbox) {
		if (hitbox.getRadius() < defaultTableObjectMaxSize) {
			defaultTable.insert(entity, hitbox, position);
		}
		largeObjectsTable.insert(entity, hitbox, position);
	});
	enemyView.each([&](auto entity, auto& enemy, auto& position, auto& hitbox) {
		if (hitbox.getRadius() < defaultTableObjectMaxSize) {
			defaultTable.insert(entity, hitbox, position);
		}
		largeObjectsTable.insert(entity, hitbox, position);
	});

	// Collision detection, looping through only players and enemies
	playerView.each([&](auto entity, auto& player, auto& position, auto& hitbox) {
		for (auto bullet : defaultTable.getNearbyObjects(hitbox, position), largeObjectsTable.getNearbyObjects(hitbox, position)) {
			// Make sure it's an enemy bullet
			if (!registry.has<EnemyBulletComponent>(bullet)) {
				continue;
			}

			auto& bulletPosition = playerBulletView.get<PositionComponent>(bullet);
			auto& bulletHitbox = playerBulletView.get<HitboxComponent>(bullet);
			if (collides(position, hitbox, bulletPosition, bulletHitbox)) {
				// TODO: collision stuff
				
				// Bullet not deleted
				// This is mostly because if the enemy bullet will spawn a bunch of other bullets later but it is deleted, 
				// the player can easily tank this one bullet to avoid having to dodge the bunch of other bullets
			}
		}
	});

	enemyView.each([&](auto entity, auto& enemy, auto& position, auto& hitbox) {
		for (auto bullet : defaultTable.getNearbyObjects(hitbox, position), largeObjectsTable.getNearbyObjects(hitbox, position)) {
			// Make sure it's a player bullet
			if (!registry.has<PlayerBulletComponent>(bullet)) {
				continue;
			}

			auto& bulletPosition = playerBulletView.get<PositionComponent>(bullet);
			auto& bulletHitbox = playerBulletView.get<HitboxComponent>(bullet);
			if (collides(position, hitbox, bulletPosition, bulletHitbox)) {
				// TODO: collision stuff

				// TODO: call entity's OnDeath if entity died and if it has an OnDeath

				// The bullet entity is not destroyed in case there are MPs that are using its MP as a reference
			}
		}
	});
}