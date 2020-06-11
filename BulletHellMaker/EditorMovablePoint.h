#pragma once
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <map>
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
#include "LevelPackObject.h"
#include "IDGenerator.h"

class EMPSpawnType;
class EditorMovablePoint;

class BulletModel : public LevelPackObject, public TextMarshallable, public std::enable_shared_from_this<BulletModel> {
public:
	inline BulletModel() {}
	inline BulletModel(int id) {
		this->id = id;
	}
	/*
	Copy constructor.
	*/
	BulletModel(std::shared_ptr<const BulletModel> copy);
	/*
	Copy constructor.
	*/
	BulletModel(const BulletModel* copy);

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<bool, std::string> legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const;

	inline Animatable getAnimatable() const { return animatable; }
	inline float getHitboxRadius() const { return hitboxRadius; }
	inline float getDespawnTime() const { return despawnTime; }
	inline float getShadowTrailInterval() const { return shadowTrailInterval; }
	inline float getShadowTrailLifespan() const { return shadowTrailLifespan; }
	inline int getDamage() const { return damage; }
	inline bool getLoopAnimation() const { return loopAnimation; }
	inline Animatable getBaseSprite() const { return baseSprite; }
	inline bool getPlaysSound() const { return playSoundOnSpawn; }
	inline SoundSettings& getSoundSettings() { return soundSettings; }

	inline void setName(std::string name) { this->name = name; onModelChange(); }
	inline void setPlaysSound(bool playsSound) { playSoundOnSpawn = playsSound; onModelChange(); }
	inline void setDamage(float damage) { this->damage = damage; onModelChange(); }
	inline void setAnimatable(Animatable animatable) { this->animatable = animatable; onModelChange(); }
	inline void setLoopAnimation(bool loopAnimation) { this->loopAnimation = loopAnimation; onModelChange(); }
	inline void setBaseSprite(Animatable baseSprite) { assert(baseSprite.isSprite()); this->baseSprite = baseSprite; onModelChange(); }
	inline void setHitboxRadius(float hitboxRadius) { this->hitboxRadius = hitboxRadius; onModelChange(); }
	inline void setDespawnTime(float despawnTime) { this->despawnTime = despawnTime; onModelChange(); }
	inline void setShadowTrailInterval(float shadowTrailInterval) { this->shadowTrailInterval = shadowTrailInterval; onModelChange(); }
	inline void setShadowTrailLifespan(float shadowTrailLifespan) { this->shadowTrailLifespan = shadowTrailLifespan; onModelChange(); }

	inline void addModelUser(std::shared_ptr<EditorMovablePoint> user) {
		modelUsers.insert(user);
	}
	void onModelChange();

private:
	// Radius of the EMP's hitbox. Set to <= 0 if the EMP is not a bullet.
	float hitboxRadius = 0;

	// The minimum of this value and the total time to complete all EMPActions is the time until this EMP despawns
	// Set to <= 0 if unused (then the total EMPActions time will be used instead)
	float despawnTime = 0;

	// See ShadowTrailComponent
	float shadowTrailInterval = 0.15f;
	// Set to 0 or a negative number to disable shadow trail
	float shadowTrailLifespan = 0;

	// Only applicable if the EMP is not a bullet (hitboxRadius <= 0)
	Animatable animatable;
	// Only applicable if animatable is an animation
	bool loopAnimation = false;
	// The animatable that will be used after the animation ends. Only necessary if animatable is an animation and loopAnimation is false
	Animatable baseSprite;

	// Only for bullets; the amount of damage this bullet deals
	int damage = 1;

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
class EditorMovablePoint : public LevelPackObject, public TextMarshallable, public std::enable_shared_from_this<EditorMovablePoint> {
public:
	/*
	Constructor only to be used for EditorMovablePoints with no parent.

	setID - whether the ID of this EMP should be set with this constructor. setID should be false if load() will be called right after.
	bulletModelsCount - reference to the map that maps the bullet model ids used by this EMP and all children EMP to the number of times
		that bullet model id is used.
	*/
	EditorMovablePoint(IDGenerator* idGen, bool setID, std::map<int, int>* bulletModelsCount);
	/*
	Constructor for EditorMovablePoints with a parent.

	bulletModelsCount - reference to the map that maps the bullet model ids used by this EMP and all children EMP to the number of times
		that bullet model id is used.
	setID - whether the ID of this EMP should be set with this constructor. setID should be false if load() will be called right after.
	*/
	EditorMovablePoint(IDGenerator* idGen, std::weak_ptr<EditorMovablePoint> parent, std::map<int, int>* bulletModelsCount, bool setID);
	/*
	Copy constructor.
	Note that this makes a deep copy of everything except idGen and bulletModelsCount, whose references are taken from copy.
	If this EMP is to be used in a different EditorAttack, onNewParentEditorAttack() should be called.
	*/
	EditorMovablePoint(std::shared_ptr<const EditorMovablePoint> copy);
	/*
	Copy constructor.
	Note that this makes a deep copy of everything except idGen and bulletModelsCount, whose references are taken from copy.
	If this EMP is to be used in a different EditorAttack, onNewParentEditorAttack() should be called.
	*/
	EditorMovablePoint(const EditorMovablePoint* copy);

	std::shared_ptr<LevelPackObject> clone() const override;


	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<bool, std::string> legal(LevelPack& levelPack, SpriteLoader& spriteLoader) const override;

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
	void removeBulletModel();

	inline bool requiresBaseSprite() {
		return !animatable.isSprite() && !loopAnimation;
	}
	inline int getID() const { return id; }
	inline Animatable getAnimatable() const { return animatable; }
	inline float getHitboxRadius() const { return hitboxRadius; }
	inline float getDespawnTime() const { return despawnTime; }
	inline const std::shared_ptr<EMPSpawnType> getSpawnType() { return spawnType; }
	inline const std::vector<std::shared_ptr<EditorMovablePoint>> getChildren() { return children; }
	inline std::shared_ptr<EMPAction> getAction(int index) { return actions[index]; }
	inline const std::vector<std::shared_ptr<EMPAction>> getActions() { return actions; }
	inline const int getActionsCount() const { return actions.size(); }
	inline float getShadowTrailInterval() const { return shadowTrailInterval; }
	inline float getShadowTrailLifespan() const { return shadowTrailLifespan; }
	inline float getTotalPathTime() const {
		float total = 0;
		for (auto action : actions) {
			total += action->getTime();
		}
		return total;
	}
	inline std::shared_ptr<EditorMovablePoint> getParent() { return parent.lock(); }
	inline int getDamage() const { return damage; }
	inline bool getLoopAnimation() const { return loopAnimation; }
	inline Animatable getBaseSprite() const { return baseSprite; }
	inline BULLET_ON_COLLISION_ACTION getOnCollisionAction() const { return onCollisionAction; }
	inline SoundSettings& getSoundSettings() { return soundSettings; }
	inline int getBulletModelID() const { return bulletModelID; }
	inline bool getInheritRadius() const { return inheritRadius; }
	inline bool getInheritDespawnTime() const { return inheritDespawnTime; }
	inline bool getInheritShadowTrailInterval() const { return inheritShadowTrailInterval; }
	inline bool getInheritShadowTrailLifespan() const { return inheritShadowTrailLifespan; }
	inline bool getInheritAnimatables() const { return inheritAnimatables; }
	inline bool getInheritDamage() const { return inheritDamage; }
	inline bool getInheritSoundSettings() const { return inheritSoundSettings; }
	inline float getPierceResetTime() const { return pierceResetTime; }
	inline bool getIsBullet() const { return isBullet; }
	inline bool usesBulletModel() const { return bulletModelID >= 0; }
	/*
	Returns the ID of every recursive child in this EMP. Does not include this EMP's ID.
	*/
	std::vector<int> getChildrenIDs() const;
	/*
	Returns whether this EMP is the main EMP of some EditorAttack.
	*/
	bool isMainEMP() const;

	void setID(int id);
	inline void setPierceResetTime(float pierceResetTime) { this->pierceResetTime = pierceResetTime; }
	inline void setOnCollisionAction(BULLET_ON_COLLISION_ACTION action) { onCollisionAction = action; }
	inline void setDamage(float damage) { this->damage = damage; }
	inline void setAnimatable(Animatable animatable) { this->animatable = animatable; }
	inline void setLoopAnimation(bool loopAnimation) { this->loopAnimation = loopAnimation; }
	inline void setBaseSprite(Animatable baseSprite) { assert(baseSprite.isSprite()); this->baseSprite = baseSprite; }
	inline void setHitboxRadius(float hitboxRadius) { this->hitboxRadius = hitboxRadius; }
	inline void setDespawnTime(float despawnTime) { this->despawnTime = despawnTime; }
	void setSpawnType(std::shared_ptr<EMPSpawnType> spawnType);
	void setSpawnTypeTime(float time);
	inline void setShadowTrailInterval(float shadowTrailInterval) { this->shadowTrailInterval = shadowTrailInterval; }
	inline void setShadowTrailLifespan(float shadowTrailLifespan) { this->shadowTrailLifespan = shadowTrailLifespan; }
	inline void setInheritRadius(bool inheritRadius, const LevelPack& levelPack) { this->inheritRadius = inheritRadius; loadBulletModel(levelPack);  }
	inline void setInheritDespawnTime(bool inheritDespawnTime, const LevelPack& levelPack) { this->inheritDespawnTime = inheritDespawnTime; }
	inline void setInheritShadowTrailInterval(bool inheritShadowTrailInterval, const LevelPack& levelPack) { this->inheritShadowTrailInterval = inheritShadowTrailInterval; loadBulletModel(levelPack); }
	inline void setInheritShadowTrailLifespan(bool inheritShadowTrailLifespan, const LevelPack& levelPack) { this->inheritShadowTrailLifespan = inheritShadowTrailLifespan; loadBulletModel(levelPack); }
	inline void setInheritAnimatables(bool inheritAnimatables, const LevelPack& levelPack) { this->inheritAnimatables = inheritAnimatables; loadBulletModel(levelPack); }
	inline void setInheritDamage(bool inheritDamage, const LevelPack& levelPack) { this->inheritDamage = inheritDamage; loadBulletModel(levelPack); }
	inline void setInheritSoundSettings(bool inheritSoundSettings, const LevelPack& levelPack) { this->inheritSoundSettings = inheritSoundSettings; loadBulletModel(levelPack); }
	inline void setIsBullet(bool isBullet) { this->isBullet = isBullet; }
	inline void setSoundSettings(SoundSettings soundSettings) { this->soundSettings = soundSettings; }
	inline void setActions(std::vector<std::shared_ptr<EMPAction>> actions) { this->actions = actions; }
	/*
	Inserts an EMPAction such that the new action is at the specified index
	*/
	void insertAction(int index, std::shared_ptr<EMPAction> action);
	void removeAction(int index);
	/*
	Replace the EMPAction at some index
	*/
	void replaceAction(int index, std::shared_ptr<EMPAction> action);
	/*
	Removes a direct child. Undefined behavior if the child doesn't exist.

	id - the ID of the child EMP
	*/
	void removeChild(int id);
	/*
	Detaches this EMP from its parent, if it has one.
	*/
	void detachFromParent();
	/*
	Creates an EMP and adds it as a child of this EMP.
	*/
	std::shared_ptr<EditorMovablePoint> createChild();
	/*
	Adds an existing EMP to the list of children.
	*/
	void addChild(std::shared_ptr<EditorMovablePoint> child);
	/*
	Should be called whenever this EMP is changed to be part of a different EditorAttack.

	newAttack - the new EditorAttack that this EMP is a child of
	*/
	void onNewParentEditorAttack(std::shared_ptr<EditorAttack> newAttack);


	/*
	Generates a list of string vectors such that, when each of the string vectors are added to a tgui::TreeView,
	the tree hierarchy of the EMPs of this attack is created.

	nodeText - a function that takes an EMP and returns a string -- the text in the tgui::TreeView for the node for that EMP
	pathToThisEmp - the ordered series of strings from the root of the tree this EMP is part of, to this EMP.
		For example, if this EMP has id 3 and its tree looks like
		      0
			1   2
			      3
		then pathToThisEmp will be { nodeText(emp0), nodeText(emp2) }
	*/
	std::vector<std::vector<sf::String>> generateTreeViewEmpHierarchy(std::function<sf::String(const EditorMovablePoint&)> nodeText, std::vector<sf::String> pathToThisEmp);
	/*
	Generates the path to this EMP such that it can be inserted as an item into a tgui::TreeView

	nodeText - a function that takes an EMP and returns a string -- the text in the tgui::TreeView for the node for that EMP
	*/
	std::vector<sf::String> generatePathToThisEmp(std::function<sf::String(const EditorMovablePoint&)> nodeText);

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
	inline std::shared_ptr<EditorMovablePoint> searchID(int id) const {
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

	/*
	For testing.
	*/
	bool operator==(const EditorMovablePoint& other) const;

private:
	// ID is unique only to the attack and is not saved

	// Points to the EMP ID generator in the EditorAttack this EMP is a child of
	IDGenerator* idGen;

	bool isBullet = true;

	// Radius of the EMP's hitbox
	float hitboxRadius = 0;

	// The minimum of this value and the total time to complete all EMPActions is the time until this EMP despawns
	// Set to <= 0 if unused (then the total EMPActions time will be used instead; calculated with getTotalPathTime())
	float despawnTime = 0;

	// This EMP's reference. Not saved
	std::weak_ptr<EditorMovablePoint> parent;
	// EMPs that have this EMP as a reference; sorted non-descending by the EMP's spawn type's time
	std::vector<std::shared_ptr<EditorMovablePoint>> children;
	// EMPActions that will be carried out in order
	std::vector<std::shared_ptr<EMPAction>> actions;
	// Details of how this EMP will be spawned. The time field of EMPSpawnType should be modified only from this class so that children's
	// ordering can be maintained. The time field in EMPSpawnType is ignored and this EMP is spawned instantly if it is the main EMP of an EditorAttack.
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

	// The amount of damage this bullet deals
	int damage = 1;

	// Only for bullets; determines what happens when the bullet makes contact with something
	BULLET_ON_COLLISION_ACTION onCollisionAction = DESTROY_THIS_BULLET_ONLY;
	// Time after hitting an enemy that the entity is able to be hit by this same bullet again; only for PIERCE_ENTITY onCollisionAction
	float pierceResetTime = 2.0f;
	
	// Sound played on this EMP spawn
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
	bool inheritSoundSettings = true;
	// ---------------------------------------------------------------

	// Reference to the map that maps the bullet model ids used by this EMP and all children EMP to the number of times
	// that bullet model id is used.
	// This map will not be saved when format() is called, but will be rebuilt in load().
	// Points to the bulletModelsCount in the EditorAttack this EMP is a child of.
	std::map<int, int>* bulletModelsCount;

	// Whether the ID marked as used in idGen was marked from this EMP; not saved in format()
	bool idResolved;

	/*
	A special version of load() intended to be used only by the copy constructor.
	copyConstructorLoad() does everything load() does, but will not modify the references to any EditorAttack's fields, and this EMP and its
	children will have ID -1. onNewParentEditorAttack() should be called on this EMP before it is added to any EditorAttack to fix its IDs and references.

	This special version of load() is required only for EMP because this class uses references to a parent EditorAttack's fields.
	*/
	void copyConstructorLoad(std::string formattedString);
	/*
	Helper function for getChildrenIDs(). Populates arr with this EMP's ID and all its
	recursive children's IDs.
	*/
	void getChildrenIDsHelper(std::vector<int>& arr) const;
	/*
	Changes this EMP's ID to be some unused ID if it is currently in use.

	recurseOnChildren - whether to call this same function on all of this EMP's children
	*/
	void resolveIDConflicts(bool recurseOnChildren);
	/*
	Recursively deletes this EMP and its children's IDs from idGen.
	*/
	void recursiveDeleteID();

	/*
	Change this EMP's ID to be some unused ID if it is currently in use.
	*/
	void setBulletModelID(int newID);
	/*
	Remove this EMP's bullet model ID from the bulletModelsCount map.
	Should be called whenever an EMP is removed from an EditorAttack.

	recurseOnChildren - whether to call this same function on all of this EMP's children
	*/
	void removeBulletModelFromBulletModelsCount(bool recurseOnChildren);
	/*
	Updates this EMP's bulletModelsCount to reflect this EMP being added to some EditorAttack.
	Should be called whenever an EMP is added to an EditorAttack.

	recurseOnChildren - whether to call this same function on all of this EMP's children
	*/
	void updateBulletModelToBulletModelsCount(bool recurseOnChildren);
};