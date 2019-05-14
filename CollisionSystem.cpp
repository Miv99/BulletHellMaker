#include "Components.h"
#include "CollisionSystem.h"
#include "Enemy.h"
#include "Level.h"
#include "Components.h"
#include "SpriteEffectAnimation.h"
#include "LevelPack.h"

float distance(float x1, float y1, float x2, float y2) {
	return sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
}

bool collides(const PositionComponent& p1, const HitboxComponent& h1, const PositionComponent& p2, const HitboxComponent& h2) {
	return distance(p1.getX() + h1.getX(), p1.getY() + h1.getY(), p2.getX() + h2.getX(), p2.getY() + h2.getY()) <= (h1.getRadius() + h2.getRadius());
}

float max(float a, float b) {
	return a > b ? a : b;
}

CollisionSystem::CollisionSystem(LevelPack& levelPack, EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry & registry, float mapWidth, float mapHeight, const HitboxComponent& largestHitbox) : levelPack(levelPack), queue(queue), spriteLoader(spriteLoader), registry(registry) {
	defaultTableObjectMaxSize = 2.0f * max(mapWidth, mapHeight) / 10.0;
	defaultTable = SpatialHashTable<uint32_t>(mapWidth, mapHeight, defaultTableObjectMaxSize/2.0f);
	largeObjectsTable = SpatialHashTable<uint32_t>(mapWidth, mapHeight, largestHitbox.getRadius() * 2.0f);
}

void CollisionSystem::update(float deltaTime) {
	auto playerView = registry.view<PlayerTag, PositionComponent, HitboxComponent>(entt::persistent_t{});
	auto enemyView = registry.view<EnemyComponent, PositionComponent, HitboxComponent>(entt::persistent_t{});
	auto playerBulletView = registry.view<PlayerBulletComponent, PositionComponent, HitboxComponent>(entt::persistent_t{});
	auto enemyBulletView = registry.view<EnemyBulletComponent, PositionComponent, HitboxComponent>(entt::persistent_t{});

	uint32_t player = registry.attachee<PlayerTag>();
	auto& playerHitbox = registry.get<HitboxComponent>(player);
	auto& playerPosition = registry.get<PositionComponent>(player);

	// Since HitboxComponent's update is only for updating hitbox disable time and only the player's hitbox can be disabled,
	// only update the player's hitbox
	playerHitbox.update(deltaTime);

	// Reinsert all bullets, players, and enemies into tables
	defaultTable.clear();
	largeObjectsTable.clear();
	playerBulletView.each([&](auto entity, auto& playerBullet, auto& position, auto& hitbox) {
		playerBullet.update(deltaTime);

		// Check hitbox size for insertion into correct table
		if (hitbox.getRadius() < defaultTableObjectMaxSize) {
			defaultTable.insert(entity, hitbox, position);
		} else {
			largeObjectsTable.insert(entity, hitbox, position);
		}
	});
	enemyBulletView.each([&](auto entity, auto& enemyBullet, auto& position, auto& hitbox) {
		enemyBullet.update(deltaTime);

		// Check hitbox size for insertion into correct table
		if (hitbox.getRadius() < defaultTableObjectMaxSize) {
			defaultTable.insert(entity, hitbox, position);
		} else {
			largeObjectsTable.insert(entity, hitbox, position);
		}
	});

	// largeObjectsTable always contains players and enemies
	// Insert player
	if (playerHitbox.getRadius() < defaultTableObjectMaxSize) {
		defaultTable.insert(player, playerHitbox, playerPosition);
	}
	largeObjectsTable.insert(player, playerHitbox, playerPosition);
	// Insert enemies
	enemyView.each([&](auto entity, auto& enemy, auto& position, auto& hitbox) {
		if (hitbox.getRadius() < defaultTableObjectMaxSize) {
			defaultTable.insert(entity, hitbox, position);
		}
		largeObjectsTable.insert(entity, hitbox, position);
	});

	// Collision detection, looping through only players and enemies
	{
		// Check for player
		
		if (!playerHitbox.isDisabled()) {
			auto all = defaultTable.getNearbyObjects(playerHitbox, playerPosition);
			auto large = largeObjectsTable.getNearbyObjects(playerHitbox, playerPosition);
			all.insert(all.end(), large.begin(), large.end());
			for (auto bullet : all) {
				// Make sure it's an enemy bullet
				if (!registry.has<EnemyBulletComponent>(bullet)) {
					continue;
				}

				auto& bulletPosition = enemyBulletView.get<PositionComponent>(bullet);
				auto& bulletHitbox = enemyBulletView.get<HitboxComponent>(bullet);
				if (registry.get<EnemyBulletComponent>(bullet).isValidCollision(player) && collides(playerPosition, playerHitbox, bulletPosition, bulletHitbox)) {
					// Player takes damage
					// Disable hitbox for invulnerability time
					float invulnTime = registry.get<PlayerTag>().getInvulnerabilityTime();
					if (invulnTime > 0) {
						playerHitbox.disable(invulnTime);
						// Player flashes white
						registry.get<SpriteComponent>(player).setEffectAnimation(std::make_unique<FlashWhiteSEA>(registry.get<SpriteComponent>(player).getSprite(), invulnTime));
					}
					if (registry.has<HealthComponent>(player) && registry.get<HealthComponent>(player).takeDamage(registry.get<EnemyBulletComponent>(bullet).getDamage())) {
						// Player is dead

						// Play death sound
						registry.get<LevelManagerTag>().getLevelPack()->playSound(registry.get<PlayerTag>().getDeathSound());

						//TODO: handle death stuff
					} else {
						registry.get<EnemyBulletComponent>(bullet).onCollision(player);

						// Play death sound
						registry.get<LevelManagerTag>().getLevelPack()->playSound(registry.get<PlayerTag>().getHurtSound());
					}

					// Handle OnCollisionAction
					switch (registry.get<EnemyBulletComponent>(bullet).getOnCollisionAction()) {
					case TURN_INTANGIBLE:
						// Remove all components except for MovementPathComponent, PositionComponent, DespawnComponent, and SpriteComponent
						if (registry.has<ShadowTrailComponent>(bullet)) {
							registry.remove<ShadowTrailComponent>(bullet);
						}
						registry.remove<HitboxComponent>(bullet);
						registry.remove<EnemyBulletComponent>(bullet);
						if (registry.has<EMPSpawnerComponent>(bullet)) {
							registry.remove<EMPSpawnerComponent>(bullet);
						}
						break;
					case DESTROY_THIS_BULLET_AND_ATTACHED_CHILDREN:
						registry.get<DespawnComponent>(bullet).setMaxTime(0);
						break;
					case DESTROY_THIS_BULLET_ONLY:
						// Remove all components except for MovementPathComponent, PositionComponent, and DespawnComponent
						if (registry.has<ShadowTrailComponent>(bullet)) {
							registry.remove<ShadowTrailComponent>(bullet);
						}
						registry.remove<SpriteComponent>(bullet);
						registry.remove<HitboxComponent>(bullet);
						registry.remove<EnemyBulletComponent>(bullet);
						if (registry.has<EMPSpawnerComponent>(bullet)) {
							registry.remove<EMPSpawnerComponent>(bullet);
						}
						break;
					case PIERCE_ENTITY:
						// Flash bullet
						auto& sprite = registry.get<SpriteComponent>(bullet);
						sprite.setEffectAnimation(std::make_unique<FlashWhiteSEA>(sprite.getSprite(), registry.get<EnemyBulletComponent>(bullet).getPierceResetTime()));
					}
				}
			}
		}
	}

	enemyView.each([&](auto entity, auto& enemy, auto& position, auto& hitbox) {
		if (!hitbox.isDisabled()) {
			auto all = defaultTable.getNearbyObjects(hitbox, position);
			auto large = largeObjectsTable.getNearbyObjects(hitbox, position);
			all.insert(all.end(), large.begin(), large.end());
			for (auto bullet : all) {
				// Make sure it's a player bullet
				if (!registry.has<PlayerBulletComponent>(bullet)) {
					continue;
				}

				auto& bulletPosition = playerBulletView.get<PositionComponent>(bullet);
				auto& bulletHitbox = playerBulletView.get<HitboxComponent>(bullet);
				if (registry.get<PlayerBulletComponent>(bullet).isValidCollision(entity) && collides(position, hitbox, bulletPosition, bulletHitbox)) {
					// Enemy takes damage
					if (registry.has<HealthComponent>(entity) && registry.get<HealthComponent>(entity).takeDamage(registry.get<PlayerBulletComponent>(bullet).getDamage())) {
						// Enemy is dead

						// Play death sound
						registry.get<LevelManagerTag>().getLevelPack()->playSound(enemy.getEnemyData()->getDeathSound());

						// Call the enemy's DeathActions
						for (std::shared_ptr<DeathAction> deathAction : enemy.getEnemyData()->getDeathActions()) {
							deathAction->execute(levelPack, queue, registry, spriteLoader, entity);
						}
						// Play death animation
						enemy.getCurrentDeathAnimationAction()->execute(levelPack, queue, registry, spriteLoader, entity);

						// Drop items, if any
						auto currentLevel = registry.get<LevelManagerTag>().getLevel();
						for (auto itemAndAmountPair : enemy.getEnemySpawnInfo().getItemsDroppedOnDeath()) {
							queue.pushBack(std::make_unique<EMPDropItemCommand>(registry, spriteLoader, position.getX(), position.getY(), itemAndAmountPair.first, itemAndAmountPair.second));
						}

						// Delete enemy
						registry.get<DespawnComponent>(entity).setMaxTime(0);
					} else {
						registry.get<PlayerBulletComponent>(bullet).onCollision(entity);

						// Play hurt sound
						registry.get<LevelManagerTag>().getLevelPack()->playSound(enemy.getEnemyData()->getHurtSound());
					}

					// Handle OnCollisionAction
					switch (registry.get<PlayerBulletComponent>(bullet).getOnCollisionAction()) {
					case TURN_INTANGIBLE:
						// Remove all components except for MovementPathComponent, PositionComponent, DespawnComponent, and SpriteComponent
						if (registry.has<ShadowTrailComponent>(bullet)) {
							registry.remove<ShadowTrailComponent>(bullet);
						}
						registry.remove<HitboxComponent>(bullet);
						registry.remove<PlayerBulletComponent>(bullet);
						if (registry.has<EMPSpawnerComponent>(bullet)) {
							registry.remove<EMPSpawnerComponent>(bullet);
						}
						break;
					case DESTROY_THIS_BULLET_AND_ATTACHED_CHILDREN:
						registry.get<DespawnComponent>(bullet).setMaxTime(0);
						break;
					case DESTROY_THIS_BULLET_ONLY:
						// Remove all components except for MovementPathComponent, PositionComponent, and DespawnComponent
						if (registry.has<ShadowTrailComponent>(bullet)) {
							registry.remove<ShadowTrailComponent>(bullet);
						}
						registry.remove<SpriteComponent>(bullet);
						registry.remove<HitboxComponent>(bullet);
						registry.remove<PlayerBulletComponent>(bullet);
						if (registry.has<EMPSpawnerComponent>(bullet)) {
							registry.remove<EMPSpawnerComponent>(bullet);
						}
						break;
					case PIERCE_ENTITY:
						// Do nothing
						break;
					}
				}
			}
		}
	});
}