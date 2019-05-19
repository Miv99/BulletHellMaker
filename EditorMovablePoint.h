#pragma once
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <set>
#include <limits>
#include "TextMarshallable.h"
#include "EditorMovablePointAction.h"
#include "EntityCreationQueue.h"
#include "Animatable.h"
#include "SpriteLoader.h"
#include "Components.h"
#include "CollisionSystem.h"
#include "AudioPlayer.h"

class EMPSpawnType;
class EditorMovablePoint;

class BulletModel : public TextMarshallable, public std::enable_shared_from_this<BulletModel> {
public:
	inline BulletModel() {}
	inline BulletModel(int id) : id(id) {}

	std::string format() override;
	void load(std::string formattedString) override;

	inline int getID() { return id; }
	inline Animatable getAnimatable() { return animatable; }
	inline float getHitboxRadius() { return hitboxRadius; }
	inline float getDespawnTime() { return despawnTime; }
	inline float getShadowTrailInterval() { return shadowTrailInterval; }
	inline float getShadowTrailLifespan() { return shadowTrailLifespan; }
	inline int getDamage() { return damage; }
	inline bool getLoopAnimation() { return loopAnimation; }
	inline Animatable getBaseSprite() { return baseSprite; }
	inline BULLET_ON_COLLISION_ACTION getOnCollisionAction() { return onCollisionAction; }
	inline bool getPlaysSound() { return playSoundOnSpawn; }
	inline SoundSettings& getSoundSettings() { return soundSettings; }

	inline void setPlaysSound(bool playsSound) { playSoundOnSpawn = playsSound; }
	inline void setOnCollisionAction(BULLET_ON_COLLISION_ACTION action) { onCollisionAction = action; }
	inline void setDamage(float damage) { this->damage = damage; }
	inline void setAnimatable(Animatable animatable) { this->animatable = animatable; }
	inline void setLoopAnimation(bool loopAnimation) { this->loopAnimation = loopAnimation; }
	inline void setBaseSprite(Animatable baseSprite) { assert(baseSprite.isSprite()); this->baseSprite = baseSprite; }
	inline void setHitboxRadius(float hitboxRadius) { this->hitboxRadius = hitboxRadius; }
	inline void setDespawnTime(float despawnTime) { this->despawnTime = despawnTime; }
	inline void setShadowTrailInterval(float shadowTrailInterval) { this->shadowTrailInterval = shadowTrailInterval; }
	inline void setShadowTrailLifespan(float shadowTrailLifespan) { this->shadowTrailLifespan = shadowTrailLifespan; }

	inline void addModelUser(std::shared_ptr<EditorMovablePoint> user) {
		modelUsers.insert(user);
	}
	void onModelChange();

private:
	int id;

	// Radius of the EMP's hitbox. Set to <= 0 if the EMP is not a bullet.
	float hitboxRadius = 0;

	// The minimum of this value and the total time to complete all EMPActions is the time until this EMP despawns
	// Set to < 0 if unused (then the total EMPActions time will be used instead)
	float despawnTime = -1;

	// See ShadowTrailComponent
	float shadowTrailInterval = 0.15f;
	// Set to 0 or a negative number to disable shadow trail
	float shadowTrailLifespan = 0;

	// Only applicable if the EMP is not a bullet (hitboxRadius <= 0)
	Animatable animatable;
	// Only applicable if animatable is an animation
	bool loopAnimation;
	// The animatable that will be used after the animation ends. Only necessary if animatable is an animation and loopAnimation is false
	Animatable baseSprite;

	// Only for bullets; the amount of damage this bullet deals
	int damage = 1;

	// Only for bullets; determines what happens when the bullet makes contact with something
	BULLET_ON_COLLISION_ACTION onCollisionAction = DESTROY_THIS_BULLET_ONLY;
	
	bool playSoundOnSpawn = false;
	SoundSettings soundSettings;

	// Set of all EMPs that use this bullet model. Not saved.
	std::set<std::shared_ptr<EditorMovablePoint>> modelUsers;
};

/*
MovablePoint used in the editor to represent bullets or bullet references.
EMP for short.

If an EMP uses a bullet model, make sure to call loadBulletModel() every time the bullet model changes.
*/
class EditorMovablePoint : public TextMarshallable, public std::enable_shared_from_this<EditorMovablePoint> {
public:
	/*
	setID - whether the ID of this EMP should be set with this constructor. setID should be false if load() will be called right after.
	*/
	inline EditorMovablePoint(int& nextID, bool setID) : nextID(nextID) {
		if (setID) {
			id = nextID++;
		}
	}
	inline EditorMovablePoint(int& nextID, std::weak_ptr<EditorMovablePoint> parent) : nextID(nextID), parent(parent) {
		id = nextID++;
	}

	std::string format() override;
	void load(std::string formattedString) override;

	bool legal(SpriteLoader& spriteLoader, std::string& message);

	/*
	Loads this EMP and its children's bullet models into the EMP, it they use models.
	*/
	void dfsLoadBulletModel(const LevelPack& levelPack);
	/*
	Loads this EMP's bullet model into the EMP, if it uses a model.
	*/
	void loadBulletModel(const LevelPack& levelPack);
	/*
	Loads a bullet model into this EMP and sets the model to be this EMP's bullet model.
	This should be the only way of setting an EMP's bullet model.
	*/
	void setBulletModel(std::shared_ptr<BulletModel> model);

	inline bool requiresBaseSprite() {
		return !animatable.isSprite() && !loopAnimation;
	}
	inline int getID() { return id; }
	inline Animatable getAnimatable() { return animatable; }
	inline float getHitboxRadius() { return hitboxRadius; }
	inline float getDespawnTime() { return despawnTime; }
	inline const std::shared_ptr<EMPSpawnType> getSpawnType() { return spawnType; }
	inline const std::vector<std::shared_ptr<EditorMovablePoint>> getChildren() { return children; }
	inline const std::vector<std::shared_ptr<EMPAction>> getActions() { return actions; }
	inline float getShadowTrailInterval() { return shadowTrailInterval; }
	inline float getShadowTrailLifespan() { return shadowTrailLifespan; }
	inline float getTotalPathTime() {
		float total = 0;
		for (auto action : actions) {
			total += action->getTime();
		}
		return total;
	}
	inline int getDamage() { return damage; }
	inline bool getLoopAnimation() { return loopAnimation; }
	inline Animatable getBaseSprite() { return baseSprite; }
	inline BULLET_ON_COLLISION_ACTION getOnCollisionAction() { return onCollisionAction; }
	inline bool getPlaysSound() { return playSoundOnSpawn; }
	inline SoundSettings& getSoundSettings() { return soundSettings; }
	inline int getBulletModelID() { return bulletModelID; }
	inline bool getInheritRadius() { return inheritRadius; }
	inline bool getInheritDespawnTime() { return inheritDespawnTime; }
	inline bool getInheritShadowTrailInterval() { return inheritShadowTrailInterval; }
	inline bool getInheritShadowTrailLifespan() { return inheritShadowTrailLifespan; }
	inline bool getInheritAnimatables() { return inheritAnimatables; }
	inline bool getInheritDamage() { return inheritDamage; }
	inline bool getInheritOnCollisionAction() { return inheritOnCollisionAction; }
	inline bool getInheritSoundSettings() { return inheritSoundSettings; }
	inline float getPierceResetTime() { return pierceResetTime; }

	inline void setPierceResetTime(float pierceResetTime) { this->pierceResetTime = pierceResetTime; }
	inline void setPlaysSound(bool playsSound) { playSoundOnSpawn = playsSound; }
	inline void setOnCollisionAction(BULLET_ON_COLLISION_ACTION action) { onCollisionAction = action; }
	inline void setDamage(float damage) { this->damage = damage; }
	inline void setAnimatable(Animatable animatable) { this->animatable = animatable; }
	inline void setLoopAnimation(bool loopAnimation) { this->loopAnimation = loopAnimation; }
	inline void setBaseSprite(Animatable baseSprite) { assert(baseSprite.isSprite()); this->baseSprite = baseSprite; }
	inline void setHitboxRadius(float hitboxRadius) { this->hitboxRadius = hitboxRadius; }
	inline void setDespawnTime(float despawnTime) { this->despawnTime = despawnTime; }
	void setSpawnType(std::shared_ptr<EMPSpawnType> spawnType);
	inline void setShadowTrailInterval(float shadowTrailInterval) { this->shadowTrailInterval = shadowTrailInterval; }
	inline void setShadowTrailLifespan(float shadowTrailLifespan) { this->shadowTrailLifespan = shadowTrailLifespan; }
	inline bool setInheritRadius(bool inheritRadius) { this->inheritRadius = inheritRadius; }
	inline bool setInheritDespawnTime(bool inheritDespawnTime) { this->inheritDespawnTime = inheritDespawnTime; }
	inline bool setInheritShadowTrailInterval(bool inheritShadowTrailInterval) { this->inheritShadowTrailInterval = inheritShadowTrailInterval; }
	inline bool setInheritShadowTrailLifespan(bool inheritShadowTrailLifespan) { this->inheritShadowTrailLifespan = inheritShadowTrailLifespan; }
	inline bool setInheritAnimatables(bool inheritAnimatables) { this->inheritAnimatables = inheritAnimatables; }
	inline bool setInheritDamage(bool inheritDamage) { this->inheritDamage = inheritDamage; }
	inline bool setInheritOnCollisionAction(bool inheritOnCollisionAction) { this->inheritOnCollisionAction = inheritOnCollisionAction; }
	inline bool setInheritSoundSettings(bool inheritSoundSettings) { this->inheritSoundSettings = inheritSoundSettings; }
	// Inserts an EMPAction such that the new action is at the specified index
	void insertAction(int index, std::shared_ptr<EMPAction> action);
	void removeAction(int index);
	/*
	Removes a child.

	id - the ID of the child EMP
	*/
	void removeChild(int id);
	/*
	Creates a child of this EMP and adds it to the list of children.

	spawnType - spawn type of the child
	Returns the child EMP
	*/
	std::shared_ptr<EditorMovablePoint> createChild(std::shared_ptr<EMPSpawnType> spawnType);
	/*
	Adds an existing EMP to the list of children.
	*/
	void addChild(std::shared_ptr<EditorMovablePoint> child);

	inline int getTreeSize() {
		int count = 1;
		for (auto child : children) {
			count += child->getTreeSize();
		}
		return count;
	}
	inline float searchLargestHitbox() const {
		float childrenMax = 0;
		for (auto emp : children) {
			childrenMax = std::max(childrenMax, emp->searchLargestHitbox());
		}
		return std::max(hitboxRadius, childrenMax);
	}
	// Search for the child of this subtree with the ID
	inline std::shared_ptr<EditorMovablePoint> searchID(int id) {
		for (auto child : children) {
			if (child->id == id) {
				return child;
			}
			auto subresult = child->searchID(id);
			if (subresult != nullptr) {
				return subresult;
			}
		}
		return nullptr;
	}

private:
	// ID is unique only to the attack. Not saved
	int id;
	int& nextID;

	// Radius of the EMP's hitbox. Set to <= 0 if the EMP is not a bullet.
	float hitboxRadius = 0;

	// The minimum of this value and the total time to complete all EMPActions is the time until this EMP despawns
	// Set to < 0 if unused (then the total EMPActions time will be used instead)
	float despawnTime = -1;

	// This EMP's reference. Not saved
	std::weak_ptr<EditorMovablePoint> parent;
	// EMPs that have this EMP as a reference; sorted non-descending by the EMP's spawn type's time
	std::vector<std::shared_ptr<EditorMovablePoint>> children;
	// EMPActions that will be carried out in order
	std::vector<std::shared_ptr<EMPAction>> actions;
	// Details of how this EMP will be spawned
	std::shared_ptr<EMPSpawnType> spawnType;

	// See ShadowTrailComponent
	float shadowTrailInterval = 0.15f;
	// Set to 0 or a negative number to disable shadow trail
	float shadowTrailLifespan = 0;

	// Only applicable if the EMP is not a bullet (hitboxRadius <= 0)
	Animatable animatable;
	// Only applicable if animatable is an animation
	bool loopAnimation;
	// The animatable that will be used after the animation ends. Only necessary if animatable is an animation and loopAnimation is false
	Animatable baseSprite;

	// Only for bullets; the amount of damage this bullet deals
	int damage = 1;

	// Only for bullets; determines what happens when the bullet makes contact with something
	BULLET_ON_COLLISION_ACTION onCollisionAction = DESTROY_THIS_BULLET_ONLY;
	// Time after hitting an enemy that the entity is able to be hit by this same bullet again; only for PIERCE_ENTITY onCollisionAction
	float pierceResetTime = 2.0f;
	
	bool playSoundOnSpawn = false;
	// Sound played on this EMP spawn, if playSoundOnSpawn is true
	SoundSettings soundSettings;

	// ID of the bullet model to use; when a bullet model changes, all EMPs using that model also change
	// Set to < 0 to not use a bullet model
	int bulletModelID = -1;
	// ----- Stuff that's only used if using a bullet model ---------
	bool inheritRadius = true;
	bool inheritDespawnTime = true;
	bool inheritShadowTrailInterval = true;
	bool inheritShadowTrailLifespan = true;
	bool inheritAnimatables = true;
	bool inheritDamage = true;
	bool inheritOnCollisionAction = true;
	bool inheritSoundSettings = true;
};