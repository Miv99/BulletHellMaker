#pragma once
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <limits>
#include "TextMarshallable.h"
#include "EditorMovablePointAction.h"
#include "EntityCreationQueue.h"

class EMPSpawnType;

/*
MovablePoint used in the editor to represent bullets.
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

	inline int getID() { return id; }
	inline std::string getSpriteName() { return spriteName; }
	inline std::string getSpriteSheetName() { return spriteSheetName; }
	inline float getHitboxRadius() { return hitboxRadius; }
	inline const std::shared_ptr<EMPSpawnType> getSpawnType() { return spawnType; }
	inline const std::vector<std::shared_ptr<EditorMovablePoint>> getChildren() { return children; }
	inline const std::vector<std::shared_ptr<EMPAction>> getActions() { return actions; }

	inline void setSpriteName(std::string spriteName, std::string spriteSheetName) { this->spriteName = spriteName; this->spriteSheetName = spriteSheetName; }
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

	std::string spriteName = "";
	std::string spriteSheetName = "";
	float hitboxRadius = 0;
};