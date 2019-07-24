#include "Components.h"
#include "CollisionSystem.h"
#include "Enemy.h"
#include "Level.h"
#include "Components.h"
#include "SpriteEffectAnimation.h"
#include "LevelPack.h"
#include <algorithm>

CollisionSystem::CollisionSystem(LevelPack& levelPack, EntityCreationQueue& queue, SpriteLoader& spriteLoader, entt::DefaultRegistry & registry, float mapWidth, float mapHeight) : levelPack(levelPack), queue(queue), spriteLoader(spriteLoader), registry(registry) {
	defaultTableObjectMaxSize = 2.0f * std::max(mapWidth, mapHeight) / 10.0;
	defaultTable = SpatialHashTable<uint32_t>(mapWidth, mapHeight, defaultTableObjectMaxSize/2.0f);
	largeObjectsTable = SpatialHashTable<uint32_t>(mapWidth, mapHeight, levelPack.searchLargestBulletHitbox() * 2.0f);
}

void CollisionSystem::update(float deltaTime) {
	auto playerView = registry.view<PlayerTag, PositionComponent, HitboxComponent>(entt::persistent_t{});
	auto enemyView = registry.view<EnemyComponent, PositionComponent, HitboxComponent>(entt::persistent_t{});
	auto playerBulletView = registry.view<PlayerBulletComponent, PositionComponent, HitboxComponent>(entt::persistent_t{});
	auto enemyBulletView = registry.view<EnemyBulletComponent, PositionComponent, HitboxComponent>(entt::persistent_t{});

	if (registry.has<PlayerTag>()) {
		uint32_t player = registry.attachee<PlayerTag>();
		auto& playerHitbox = registry.get<HitboxComponent>(player);
		auto& playerPosition = registry.get<PositionComponent>(player);

		// Since HitboxComponent's update is only for updating hitbox disable time and only the player's hitbox can be disabled,
		// only update the player's hitbox
		playerHitbox.update(deltaTime);

		// largeObjectsTable always contains players and enemies
		// Insert player
		if (playerHitbox.getRadius() < defaultTableObjectMaxSize) {
			defaultTable.insert(player, playerHitbox, playerPosition);
		}
		largeObjectsTable.insert(player, playerHitbox, playerPosition);
	}

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

	// Insert enemies
	enemyView.each([&](auto entity, auto& enemy, auto& position, auto& hitbox) {
		if (hitbox.getRadius() < defaultTableObjectMaxSize) {
			defaultTable.insert(entity, hitbox, position);
		}
		largeObjectsTable.insert(entity, hitbox, position);
	});

	// Collision detection, looping through only players and enemies
	if (registry.has<PlayerTag>()) {
		uint32_t player = registry.attachee<PlayerTag>();
		auto& playerHitbox = registry.get<HitboxComponent>(player);
		auto& playerPosition = registry.get<PositionComponent>(player);

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

				// This is to prevent the player from taking multiple hits in the same frame
				// We can break instead of continuing because if player hitbox is disabled, no other bullet can hit the player so just stop collision checking
				if (playerHitbox.isDisabled()) {
					break;
				}

				auto& bulletPosition = enemyBulletView.get<PositionComponent>(bullet);
				auto& bulletHitbox = enemyBulletView.get<HitboxComponent>(bullet);
				// Note: No DeathComponent::isMarkedForDeath() check here because player does not despawn on death
				if (registry.get<EnemyBulletComponent>(bullet).isValidCollision(player) && !bulletHitbox.isDisabled() && collides(playerPosition, playerHitbox, bulletPosition, bulletHitbox)) {
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
						// if player despawns, make sure to add isMarkedForDeath() check (see above comment)
					} else {
						registry.get<EnemyBulletComponent>(bullet).onCollision(player);

						// Play death sound
						registry.get<LevelManagerTag>().getLevelPack()->playSound(registry.get<PlayerTag>().getHurtSound());
					}

					// Handle OnCollisionAction
					switch (registry.get<EnemyBulletComponent>(bullet).getOnCollisionAction()) {
					case DESTROY_THIS_BULLET_AND_ATTACHED_CHILDREN:
						registry.get<DespawnComponent>(bullet).setMaxTime(0);
						break;
					case DESTROY_THIS_BULLET_ONLY:
						// Remove all components except for MovementPathComponent, PositionComponent, and DespawnComponent
						// Hitbox is simply disabled indefinitely in case other stuff uses its hitbox
						registry.get<HitboxComponent>(bullet).disable(9999999999);

						if (registry.has<ShadowTrailComponent>(bullet)) {
							registry.remove<ShadowTrailComponent>(bullet);
						}
						registry.remove<SpriteComponent>(bullet);
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
				if (!registry.get<DespawnComponent>(entity).isMarkedForDespawn() && registry.get<PlayerBulletComponent>(bullet).isValidCollision(entity) && !bulletHitbox.isDisabled() && collides(position, hitbox, bulletPosition, bulletHitbox)) {
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
					case DESTROY_THIS_BULLET_AND_ATTACHED_CHILDREN:
						registry.get<DespawnComponent>(bullet).setMaxTime(0);
						break;
					case DESTROY_THIS_BULLET_ONLY:
						// Remove all components except for MovementPathComponent, PositionComponent, and DespawnComponent
						// Hitbox is simply disabled indefinitely in case other stuff uses its hitbox
						registry.get<HitboxComponent>(bullet).disable(9999999999);

						if (registry.has<ShadowTrailComponent>(bullet)) {
							registry.remove<ShadowTrailComponent>(bullet);
						}
						registry.remove<SpriteComponent>(bullet);
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