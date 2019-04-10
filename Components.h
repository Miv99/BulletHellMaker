#pragma once
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <memory>
#include <queue>
#include <vector>
#include <math.h>
#include <cassert>
#include <string>
#include "SpriteLoader.h"
#include "SpriteEffectAnimation.h"
#include "Animation.h"
#include "Constants.h"
#include "EntityAnimatableSet.h"
#include "Animatable.h"
#include "EnemySpawn.h"

class MovablePoint;
class AggregatorMP;
class LevelPack;
class EMPAction;
class EditorAttackPattern;
class EditorEnemyPhase;
class EditorEnemy;
class EMPSpawnType;
class Level;
class EntityCreationQueue;
class EditorMovablePoint;
class Item;

class PositionComponent {
public:
	PositionComponent(float x = 0, float y = 0) : x(x), y(y) {}

	inline float getX() const { return x; }
	inline float getY() const { return y; }
	inline void setX(float x) { this->x = x; }
	inline void setY(float y) { this->y = y; }
	void setPosition(sf::Vector2f position) { x = position.x; y = position.y; }

private:
	float x;
	float y;
};

class MovementPathComponent {
public:
	/*
	entity - the entity that the entity with this component should be attached to, if any
	*/
	inline MovementPathComponent(EntityCreationQueue& queue, uint32_t self, entt::DefaultRegistry& registry, uint32_t entity, std::shared_ptr<EMPSpawnType> spawnType, std::vector<std::shared_ptr<EMPAction>> actions, float initialTime) : actions(actions), time(initialTime) {
		initialSpawn(registry, entity, spawnType, actions);
		update(queue, registry, self, registry.get<PositionComponent>(self), 0);
	}

	/*
	Updates elapsed time and updates the entity's position along its path
	*/
	void update(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, PositionComponent& entityPosition, float deltaTime);

	/*
	Returns this entity's position some time ago.
	*/
	sf::Vector2f getPreviousPosition(entt::DefaultRegistry& registry, float secondsAgo) const;

	bool usesReferenceEntity() { return useReferenceEntity; }
	uint32_t getReferenceEntity() const { return referenceEntity; }
	/*
	Sets the reference entity of this entity.
	The PositionComponent of this entity should be updated after this call.
	*/
	inline void setReferenceEntity(uint32_t reference) {
		useReferenceEntity = true;
		referenceEntity = reference;
	}
	inline std::shared_ptr<MovablePoint> getPath() { return path; }
	inline float getTime() { return time; }

	/*
	Change the path of an entity.

	timeLag - number of seconds ago that the path change should have happened
	*/
	void setPath(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, PositionComponent& entityPosition, std::shared_ptr<MovablePoint> newPath, float timeLag);

private:
	bool useReferenceEntity;
	uint32_t referenceEntity;
	// Elapsed time since the the last path change
	float time;
	std::shared_ptr<MovablePoint> path;
	// Sorted descending in age (index 0 is the oldest path).
	std::vector<std::shared_ptr<MovablePoint>> previousPaths;
	// Actions to be carried out in order; each one changes pushes back an MP to path
	std::vector<std::shared_ptr<EMPAction>> actions;
	int currentActionsIndex = 0;

	void initialSpawn(entt::DefaultRegistry& registry, uint32_t entity, std::shared_ptr<EMPSpawnType> spawnType, std::vector<std::shared_ptr<EMPAction>>& actions);
};

class HealthComponent {
public:
	HealthComponent(int health = 0, int maxHealth = 0) : health(health), maxHealth(maxHealth) {}

	/*
	Take damage and returns true if health goes below 0.
	*/
	inline bool takeDamage(int damage) {
		health -= damage;
		return health <= 0;
	}

	inline void heal(int amount) {
		health += amount;
		if (health > maxHealth) {
			health = maxHealth;
		}
	}

	inline int getHealth() const { return health; }
	inline int getMaxHealth() const { return maxHealth; }
	inline void setHealth(int health) { this->health = health; }
	inline void setMaxHealth(int maxHealth) { this->maxHealth = maxHealth; }

private:
	int health;
	int maxHealth;
};

/*
Hitbox is a single circle
*/
class HitboxComponent {
public:
	/*
	radius - hitbox radius
	x - local offset of hitbox
	y - local offset of hitbox
	*/
	HitboxComponent(ROTATION_TYPE rotationType, float radius, float x, float y) : rotationType(rotationType), radius(radius), x(x), y(y), unrotatedX(x), unrotatedY(y) {}
	/*
	radius - hitbox radius
	sprite - this entity's sprite at the time of this HitboxComponent construction
	*/
	HitboxComponent(float radius, std::shared_ptr<sf::Sprite> sprite) : radius(radius) {
		rotate(sprite);

		// Rotate sprite to angle 0 to find unrotatedX/Y, then rotate sprite back
		auto oldAngle = sprite->getRotation();
		sprite->setRotation(0);
		auto transformed = sprite->getTransform().transformPoint(sprite->getOrigin());
		unrotatedX = transformed.x;
		unrotatedY = transformed.y;
		sprite->setRotation(oldAngle);
	}

	/*
	angle - radians in range [-pi, pi]
	*/
	inline void rotate(float angle) {
		if (unrotatedX == unrotatedY == 0) return;

		if (rotationType == ROTATE_WITH_MOVEMENT) {
			float sin = std::sin(angle);
			float cos = std::cos(angle);
			x = unrotatedX * cos - unrotatedY * sin;
			y = unrotatedX * sin + unrotatedY * cos;
		} else if (rotationType == LOCK_ROTATION) {
			// Do nothing
		} else if (rotationType == LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT) {
			// Flip across y-axis if facing left
			if (angle < -PI / 2.0f || angle > PI / 2.0f) {
				y = -unrotatedY;
			} else if (angle > -PI / 2.0f && angle < PI / 2.0f) {
				y = unrotatedY;
			}
			// Do nothing (maintain last value) if angle is a perfect 90 or -90 degree angle
		}
	}
	/*
	Match origin to sprite's origin.
	*/
	inline void rotate(std::shared_ptr<sf::Sprite> sprite) {
		if (unrotatedX == unrotatedY == 0) return;

		auto oldPos = sprite->getPosition();
		sprite->setPosition(0, 0);
		auto transformed = sprite->getTransform().transformPoint(sprite->getOrigin());
		x = transformed.x;
		y = transformed.y;
		sprite->setPosition(oldPos);
	}

	inline float getRadius() const { return radius; }
	inline float getX() const { return x; }
	inline float getY() const { return y; }
	inline void setRadius(float radius) { this->radius = radius; }

private:
	// If entity has a SpriteComponent, this rotationType must match its SpriteComponent's rotationType
	ROTATION_TYPE rotationType;

	float radius;
	float x = 0, y = 0;
	// Local offset of hitbox when rotated at angle 0
	float unrotatedX, unrotatedY;
};

class SpriteComponent {
public:
	/*
	renderLayer - the layer to render this sprite at. Higher means being drawn on top.
	sublayerr - determines order of sprite drawing for sprites in the same layer. Higher means being drawn on top.
	*/
	inline SpriteComponent(int renderLayer, float subLayer) : renderLayer(renderLayer), subLayer(subLayer) {}
	/*
	loopAnimatable - only applicable if animatable is an animation
	*/
	inline SpriteComponent(SpriteLoader& spriteLoader, Animatable animatable, bool loopAnimatable, int renderLayer, float subLayer) : renderLayer(renderLayer), subLayer(subLayer) {
		setAnimatable(spriteLoader, animatable, loopAnimatable);
		if (animatable.isSprite()) {
			originalSprite = *sprite;
		}
	}
	inline SpriteComponent(ROTATION_TYPE rotationType, std::shared_ptr<sf::Sprite> sprite, int renderLayer, float subLayer) : renderLayer(renderLayer), subLayer(subLayer), 
		rotationType(rotationType), sprite(sprite), originalSprite(*sprite) {}

	void update(float deltaTime);

	/*
	angle - radians in range [-pi, pi]
	*/
	inline void rotate(float angle) { rotationAngle = angle; }

	inline int getRenderLayer() const { return renderLayer; }
	inline float getSubLayer() const { return subLayer; }
	inline bool animationIsDone() { assert(animation != nullptr); return animation->isDone(); }
	inline const std::shared_ptr<sf::Sprite> getSprite() { return sprite; }

	/*
	loopAnimatable - only applicable if animatable is an animation
	*/
	inline void setAnimatable(SpriteLoader& spriteLoader, Animatable animatable, bool loopAnimatable) {
		this->rotationType = animatable.getRotationType();

		if (animatable.isSprite()) {
			// Cancel current animation
			setAnimation(nullptr);
			updateSprite(spriteLoader.getSprite(animatable.getAnimatableName(), animatable.getSpriteSheetName()));
		} else {
			setAnimation(spriteLoader.getAnimation(animatable.getAnimatableName(), animatable.getSpriteSheetName(), loopAnimatable));
		}
	}
	inline void setEffectAnimation(std::unique_ptr<SpriteEffectAnimation> effectAnimation) { this->effectAnimation = std::move(effectAnimation); }
	// Angle in degrees
	inline void setRotation(float angle) { sprite->setRotation(angle); }
	inline void setScale(float x, float y) { sprite->setScale(x, y); }
	inline bool usesShader() {
		if (effectAnimation == nullptr) {
			return false;
		}
		return effectAnimation->usesShader();
	}
	inline sf::Shader& getShader() {
		assert(effectAnimation != nullptr);
		return effectAnimation->getShader();
	}
	inline ROTATION_TYPE getRotationType() { return rotationType; }
	/*
	Returns the angle of rotation of the sprite for the purpose of
	creating sprites with the same rotation angle and type.
	*/
	inline float getInheritedRotationAngle() { 
		if (rotationType == LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT) {
			if (lastFacedRight) return 0;
			return -PI;
		}
		return rotationAngle;
	}

private:
	int renderLayer;
	float subLayer;

	// If entity has a HitboxComponent, this rotationType must match its HitboxComponent's rotationType
	ROTATION_TYPE rotationType;
	// Last faced direction used only for rotation type LOCK_ROTATION_AND_FACE_HORIZONTAL_MOVEMENT
	bool lastFacedRight = true;
	// In radians
	float rotationAngle = 0;

	std::shared_ptr<sf::Sprite> sprite;
	// The original sprite. Used for returning to original appearance after an animation ends.
	sf::Sprite originalSprite;
	// Effect animation that the sprite is currently undergoing, if any
	std::unique_ptr<SpriteEffectAnimation> effectAnimation;
	// Animation that the sprite is currently undergoing, if any
	std::unique_ptr<Animation> animation;

	inline void updateSprite(sf::Sprite newSprite) { *sprite = newSprite; }
	inline void updateSprite(std::shared_ptr<sf::Sprite> newSprite) {
		if (!sprite) {
			// SpriteEffectAnimations can change SpriteComponent's Sprite, so create a new Sprite object to avoid 
			// accidentally modifying the parameter Sprite pointer's object
			sprite = std::make_shared<sf::Sprite>(*newSprite);
			if (effectAnimation != nullptr) {
				effectAnimation->setSpritePointer(sprite);
			}
		} else {
			updateSprite(*newSprite);
		}
	}
	inline void setAnimation(std::unique_ptr<Animation> animation) {
		if (animation) {
			this->animation = std::move(animation);
		} else {
			this->animation = nullptr;
		}
		update(0);
	}
};

class PlayerTag {
public:
	PlayerTag(float speed, float focusedSpeed, std::shared_ptr<EditorAttackPattern> attackPattern, float attackPatternLoopDelay, std::shared_ptr<EditorAttackPattern> focusedAttackPattern, float focusedAttackPatternLoopDelay);

	inline float getSpeed() { return speed; }
	inline float getFocusedSpeed() { return focusedSpeed; }
	inline std::shared_ptr<EditorAttackPattern> getAttackPattern() { return attackPattern; }
	inline std::shared_ptr<EditorAttackPattern> getFocusedAttackPattern() { return focusedAttackPattern; }
	inline float getAttackPatternTotalTime() { return attackPatternTotalTime; }
	inline float getFocusedAttackPatternTotalTime() { return focusedAttackPatternTotalTime; }

private:
	float speed;
	float focusedSpeed;
	std::shared_ptr<EditorAttackPattern> attackPattern;
	// Total time for every attack to execute in addition to the loop delay
	float attackPatternTotalTime;
	std::shared_ptr<EditorAttackPattern> focusedAttackPattern;
	float focusedAttackPatternTotalTime;
};

class EnemyComponent {
public:
	EnemyComponent(std::shared_ptr<EditorEnemy> enemyData, EnemySpawnInfo spawnInfo, int enemyID);
	void update(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, uint32_t entity, float deltaTime);

	inline float getTimeSinceSpawned() const { return timeSinceSpawned; }
	inline float getTimeSinceLastPhase() const { return timeSincePhase; }
	inline std::shared_ptr<EditorEnemy> getEnemyData() const { return enemyData; }
	inline EnemySpawnInfo getEnemySpawnInfo() const { return spawnInfo; }
	/*
	Returns the PlayAnimatableDeathAction associated with the current phase.
	*/
	std::shared_ptr<DeathAction> getCurrentDeathAnimationAction();

private:
	// Time since being spawned
	float timeSinceSpawned = 0;
	// Time since start of the current phase
	float timeSincePhase = 0;
	// Time since start of the current attack pattern
	float timeSinceAttackPattern = 0;

	EnemySpawnInfo spawnInfo;

	int enemyID;
	std::shared_ptr<EditorEnemy> enemyData;
	std::shared_ptr<EditorEnemyPhase> currentPhase = nullptr;
	std::shared_ptr<EditorAttackPattern> currentAttackPattern = nullptr;

	// Current phase index in list of phases in EditorEnemy
	int currentPhaseIndex = -1;
	// Current attack pattern index in list of attack patterns in current EditorEnemyPhase
	int currentAttackPatternIndex = -1;
	// Current attack index in list of attacks in current EditorAttackPattern
	int currentAttackIndex = -1;

	// Check for any missed phases/attack patterns/attacks since the last update and then executes their relevant actions
	void checkPhases(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, uint32_t entity);
	void checkAttackPatterns(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, uint32_t entity);
	void checkAttacks(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, uint32_t entity);
};

class DespawnComponent {
public:
	/*
	Empty DespawnComponent that does nothing on updates
	*/
	inline DespawnComponent() : attachedToEntity(false), useTime(false) {}
	/*
	maxTime - time before entity with this component despawns
	*/
	inline DespawnComponent(float maxTime) : maxTime(maxTime), useTime(true) {}
	/*
	entity - the entity that, when it despawns, the entity with this component will despawn. This entity must have a DespawnComponent.
	self - the entity with this DespawnComponent
	*/
	inline DespawnComponent(entt::DefaultRegistry& registry, uint32_t entity, uint32_t self) : attachedTo(entity), attachedToEntity(true), useTime(false) {
		assert(registry.has<DespawnComponent>(entity));
		registry.get<DespawnComponent>(entity).addChild(self);
	}

	// Returns true if entity with this component should be despawned
	inline bool update(const entt::DefaultRegistry& registry, float deltaTime) {
		bool despawn = false;
		if (attachedToEntity) {
			if (!registry.valid(attachedTo)) despawn = true;
		}
		if (useTime) {
			time += deltaTime;
			if (time >= maxTime) despawn = true;
		}
		return despawn;
	}
	
	inline void addChild(uint32_t child) { children.push_back(child); }

	inline const std::vector<uint32_t> getChildren() { return children; }
	inline void setMaxTime(float maxTime) {
		useTime = true;
		this->maxTime = maxTime;
	}

private:
	// Time since the entity with this component spawned
	float time = 0;
	float maxTime;
	bool useTime = false;

	bool attachedToEntity = false;
	uint32_t attachedTo;

	// Entities that are attached to the entity with this DespawnComponent
	// When this entity is deleted, so are all its children
	std::vector<uint32_t> children;
};

/*
Component assigned only to a single entity - the level manager.
The level manager entity is destroyed and re-created on the start of a new level.
*/
class LevelManagerTag {
public:
	inline LevelManagerTag(std::shared_ptr<Level> level) : level(level) {}
	void update(EntityCreationQueue& queue, SpriteLoader& spriteLoader, const LevelPack& levelPack, entt::DefaultRegistry& registry, float deltaTime);

	inline float getTimeSinceStartOfLevel() { return timeSinceStartOfLevel; }
	inline float getTimeSinceLastEnemySpawn() { return timeSinceLastEnemySpawn; }
	inline int getPoints() { return points; }
	inline std::shared_ptr<Level> getLevel() { return level; }

	inline void setTimeSinceLastEnemySpawn(float timeSinceLastEnemySpawn) { this->timeSinceLastEnemySpawn = timeSinceLastEnemySpawn; }

	inline void addPoints(int amount) { points += amount; }
	inline void subtractPoints(int amount) { points -= amount; if (points < 0) points = 0; }

private:
	// Time since the start of the level
	float timeSinceStartOfLevel = 0;
	// Time since the last enemy spawn
	// If no enemies have been spawned yet, this is the same as timeSinceStartOfLevel
	float timeSinceLastEnemySpawn = 0;

	std::shared_ptr<Level> level;
	// Current index in list of groups of enemies to be spawned from the level
	int currentEnemyGroupSpawnIndex = -1;

	// Points earned so far
	int points = 0;
};

class EnemyBulletComponent {
public:
	EnemyBulletComponent(int attackID, int attackPatternID, int enemyID, int enemyPhaseID) : attackID(attackID), attackPatternID(attackPatternID), enemyID(enemyID), enemyPhaseID(enemyPhaseID) {}

private:
	// The attack this bullet belongs to
	int attackID;
	// The attack pattern this bullet belongs to
	int attackPatternID;
	// The enemy this bullet belongs to
	int enemyID;
	// The enemy phase this bullet belongs to
	int enemyPhaseID;
};

class PlayerBulletComponent {
public:
	PlayerBulletComponent(int attackID, int attackPatternID, float damage) : attackID(attackID), attackPatternID(attackPatternID), damage(damage) {}

	inline float getDamage() { return damage; }

private:
	// The attack this bullet belongs to
	int attackID;
	// The attack pattern this bullet belongs to
	int attackPatternID;

	float damage;
};

/*
Component for entities that act only as a reference point for a single MovablePoint.
*/
class SimpleEMPReferenceComponent {

};

/*
Component for entities that will spawn enemy/player bullets after some time.
*/
class EMPSpawnerComponent {
public:
	/*
	Constructor for an enemy bullets spawner.

	emps - the EMPs that will be spawned by this entity and, if applicable, will be spawned with respect to parent; must be sorted ascending by time of spawn
	parent - the entity that the spawned EMPs will be spawned in reference to; may be unused depending on the EMP's spawn type
	attackID - the ID of the attack each EMP originated from
	attackPattern - same but attack pattern
	enemyID - same but enemy
	enemyPhaseID - same but enemy phase
	playAttackAnimation - whether or not to play this entity's attack animation
	*/
	EMPSpawnerComponent(std::vector<std::shared_ptr<EditorMovablePoint>> emps, uint32_t parent, int attackID, int attackPatternID, int enemyID, int enemyPhaseID, bool playAttackAnimation);
	/*
	Constructor for an player bullets spawner.

	emps - the EMPs that will be spawned by this entity and, if applicable, will be spawned with respect to parent; must be sorted ascending by time of spawn
	parent - the entity that the spawned EMPs will be spawned in reference to; may be unused depending on the EMP's spawn type
	attackID - the ID of the attack each EMP originated from
	attackPattern - same but attack pattern
	playAttackAnimation - whether or not to play this entity's attack animation
	*/
	EMPSpawnerComponent(std::vector<std::shared_ptr<EditorMovablePoint>> emps, uint32_t parent, int attackID, int attackPatternID, bool playAttackAnimation);

	void EMPSpawnerComponent::update(entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, EntityCreationQueue& queue, float deltaTime);

private:
	bool isEnemyBulletSpawner;

	std::queue<std::shared_ptr<EditorMovablePoint>> emps;
	uint32_t parent;
	int attackID;
	int attackPatternID;
	int enemyID;
	int enemyPhaseID;
	bool playAttackAnimation;

	// Time since this entity was spawned
	float time = 0;
};

/*
Component for entities with shadow trails.
*/
class ShadowTrailComponent {
public:
	/*
	interval - time inbetween each shadow's creation
	lifespan - lifespan of each shadow
	*/
	inline ShadowTrailComponent(float interval, float lifespan) : interval(interval), lifespan(lifespan) {}
	/*
	Returns true if a shadow should be created at the moment of the update call.
	*/
	inline bool update(float deltaTime) {
		if (lifespan <= 0) return false;

		time += deltaTime;
		if (time > interval) {
			time -= interval;
			return true;
		}
		return false;
	}
	inline float getLifespan() { return lifespan; }

	inline void setInterval(float interval) { this->interval = interval; time = 0; }
	inline void setLifespan(float lifespan) { this->lifespan = lifespan; time = 0; }

private:
	// Time inbetween each shadow's creation
	float interval;
	// Time since the last shadow was created
	float time = 0;
	// Lifespan of each shadow
	float lifespan;
};

/*
Component for entities with an EntityAnimatableSet
*/
class AnimatableSetComponent {
public:
	inline AnimatableSetComponent() {}
	inline AnimatableSetComponent(EntityAnimatableSet animatableSet) : animatableSet(animatableSet) {}

	// States
	static const int IDLE = 0, MOVEMENT = 1, ATTACK = 2;

	inline EntityAnimatableSet getAnimatableSet() { return animatableSet; }
	inline void setAnimatableSet(EntityAnimatableSet animatableSet) { this->animatableSet = animatableSet; }

	/*
	x, y - this entity's current global position
	spriteComponent - this entity's sprite component
	*/
	void update(SpriteLoader& spriteLoader, float x, float y, SpriteComponent& spriteComponent, float deltaTime);
	void changeState(int newState, SpriteLoader& spriteLoader, SpriteComponent &spriteComponent);

private:
	EntityAnimatableSet animatableSet;

	float lastX;
	float lastY;
	int currentState = -1;
	bool firstUpdateHasBeenCalled = false;
	int queuedState = -1;
};

/*
Component for items that can be collected by the player.
Entities with this component must also have HitboxComponent, PositionComponent, and MovementPathComponent (which does not need to have a path).
*/
class CollectibleComponent {
public:
	inline CollectibleComponent(std::shared_ptr<Item> item, float activationRadius) : item(item), activationRadius(activationRadius) {}

	void update(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, const PositionComponent& entityPos, const HitboxComponent& entityHitbox);

private:
	bool activated = false;
	std::shared_ptr<Item> item;
	float activationRadius;

	float distance(float x1, float y1, float x2, float y2) {
		return sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
	}

	bool collides(const PositionComponent& p1, const HitboxComponent& h1, const PositionComponent& p2, const HitboxComponent& h2, float hitboxExtension) {
		return distance(p1.getX() + h1.getX(), p1.getY() + h1.getY(), p2.getX() + h2.getX(), p2.getY() + h2.getY()) <= (h1.getRadius() + h2.getRadius() + hitboxExtension);
	}
};