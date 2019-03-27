#pragma once
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <limits>
#include "TextMarshallable.h"
#include "EditorMovablePointAction.h"
#include "EntityCreationQueue.h"
#include "EntityAnimatableSet.h"
#include "SpriteLoader.h"

class EMPSpawnType;

/*
MovablePoint used in the editor to represent bullets or bullet references.
EMP for short.
*/
class EditorMovablePoint : public TextMarshallable, public std::enable_shared_from_this<EditorMovablePoint> {
public:
	inline EditorMovablePoint(int& nextID) : nextID(nextID) {
		id = nextID++;
	}
	inline EditorMovablePoint(int& nextID, std::weak_ptr<EditorMovablePoint> parent) : nextID(nextID), parent(parent) {
		id = nextID++;
	}

	std::string format() override;
	void load(std::string formattedString) override;

	bool legal(SpriteLoader& spriteLoader, std::string& message);

	inline int getID() { return id; }
	inline Animatable getAnimatable() { return animatable; }
	inline float getHitboxRadius() { return hitboxRadius; }
	inline const std::shared_ptr<EMPSpawnType> getSpawnType() { return spawnType; }
	inline const std::vector<std::shared_ptr<EditorMovablePoint>> getChildren() { return children; }
	inline const std::vector<std::shared_ptr<EMPAction>> getActions() { return actions; }
	inline float getShadowTrailInterval() { return shadowTrailInterval; }
	inline float getShadowTrailLifespan() { return shadowTrailLifespan; }

	inline void setAnimatable(Animatable animatable) { this->animatable = animatable; }
	inline void setLoopAnimation(bool loopAnimation) { this->loopAnimation = loopAnimation; }
	inline void setBaseSprite(Animatable baseSprite) { assert(baseSprite.isSprite()); this->baseSprite = baseSprite; }
	inline void setHitboxRadius(float hitboxRadius) { this->hitboxRadius = hitboxRadius; }
	void setSpawnType(std::shared_ptr<EMPSpawnType> spawnType);
	// Inserts an EMPAction such that the new action is at the specified index
	void insertAction(int index, std::shared_ptr<EMPAction> action);
	void removeAction(int index);
	// Adds a child EMP
	void addChild(std::shared_ptr<EditorMovablePoint> child);

	inline int getTreeSize() {
		int count = 1;
		for (auto child : children) {
			count += child->getTreeSize();
		}
		return count;
	}
	inline float searchLargestHitbox() {
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
	// The sprite that will be used after the animation ends. Only necessary if animatable is an animation and loopAnimation is false
	Animatable baseSprite;

	// Radius of the EMP's hitbox. Set to <= 0 if the EMP is not a bullet.
	float hitboxRadius = 0;
};