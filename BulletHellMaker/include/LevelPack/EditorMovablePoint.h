#pragma once
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <map>
#include <set>
#include <limits>

#include <DataStructs/IDGenerator.h>
#include <DataStructs/SpriteLoader.h>
#include <LevelPack/Animatable.h>
#include <LevelPack/EditorMovablePointAction.h>
#include <LevelPack/LayerRootLevelPackObject.h>
#include <LevelPack/TextMarshallable.h>
#include <Game/EntityCreationQueue.h>
#include <Game/Systems/CollisionSystem.h>
#include <Game/AudioPlayer.h>
#include <Util/json.hpp>

class EMPSpawnType;
class EditorMovablePoint;

class BulletModel : public LayerRootLevelPackObject, public std::enable_shared_from_this<BulletModel> {
public:
	BulletModel();
	BulletModel(int id);
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

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	inline Animatable getAnimatable() const { return animatable; }
	inline float getHitboxRadius() const { return hitboxRadius; }
	inline float getDespawnTime() const { return despawnTime; }
	inline float getShadowTrailInterval() const { return shadowTrailInterval; }
	inline float getShadowTrailLifespan() const { return shadowTrailLifespan; }
	inline int getDamage() const { return damage; }
	inline float getPierceResetTime() const { return pierceResetTime; }
	inline bool getLoopAnimation() const { return loopAnimation; }
	inline Animatable getBaseSprite() const { return baseSprite; }
	inline bool getPlaysSound() const { return playSoundOnSpawn; }
	inline SoundSettings& getSoundSettings() { return soundSettings; }

	void setName(std::string name);
	void setPlaysSound(bool playsSound);
	void setDamage(int damage);
	void setPierceResetTime(float pierceResetTime);
	void setAnimatable(Animatable animatable);
	void setLoopAnimation(bool loopAnimation);
	void setBaseSprite(Animatable baseSprite);
	void setHitboxRadius(float hitboxRadius);
	void setDespawnTime(float despawnTime);
	void setShadowTrailInterval(float shadowTrailInterval);
	void setShadowTrailLifespan(float shadowTrailLifespan);

	inline void addModelUser(std::shared_ptr<EditorMovablePoint> user) {
		modelUsers.insert(user);
	}

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

	// Time after hitting an enemy that the entity is able to be hit by this same bullet again; only for bullets with PIERCE_ENTITY onCollisionAction
	float pierceResetTime = 2.0f;

	bool playSoundOnSpawn = false;
	SoundSettings soundSettings;

	// Set of all EMPs that use this bullet model. Not saved.
	std::set<std::shared_ptr<EditorMovablePoint>> modelUsers;

	void onModelChange();
};

/*
MovablePoint used in the editor to represent bullets or bullet references.
EMP for short.

If an EMP uses a bullet model, make sure to call loadBulletModel() every time the bullet model changes.
*/
class EditorMovablePoint : public LevelPackObject, public std::enable_shared_from_this<EditorMovablePoint> {
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

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

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
	inline float getHitboxRadius() const { return hitboxRadiusExprCompiledValue; }
	inline std::string getRawHitboxRadius() const { return hitboxRadius; }
	inline float getDespawnTime() const { return despawnTime; }
	inline const std::shared_ptr<EMPSpawnType> getSpawnType() const { return spawnType; }
	inline const std::vector<std::shared_ptr<EditorMovablePoint>> getChildren() const { return children; }
	inline std::shared_ptr<EMPAction> getAction(int index) { return actions[index]; }
	inline const std::vector<std::shared_ptr<EMPAction>> getActions() { return actions; }
	inline const int getActionsCount() const { return actions.size(); }
	inline float getShadowTrailInterval() const { return shadowTrailIntervalExprCompiledValue; }
	inline float getShadowTrailLifespan() const { return shadowTrailLifespanExprCompiledValue; }
	inline std::string getRawShadowTrailInterval() const { return shadowTrailInterval; }
	inline std::string getRawShadowTrailLifespan() const { return shadowTrailLifespan; }
	float getTotalPathTime() const;
	inline std::shared_ptr<EditorMovablePoint> getParent() const { return parent.lock(); }
	inline int getDamage() const { return damageExprCompiledValue; }
	inline std::string getRawDamage() const { return damage; }
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
	inline bool getInheritPierceResetTime() const { return inheritPierceResetTime; }
	inline bool getInheritSoundSettings() const { return inheritSoundSettings; }
	inline float getPierceResetTime() const { return pierceResetTimeExprCompiledValue; }
	inline std::string getRawPierceResetTime() const { return pierceResetTime; }
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
	inline void setPierceResetTime(std::string pierceResetTime) { this->pierceResetTime = pierceResetTime; }
	inline void setOnCollisionAction(BULLET_ON_COLLISION_ACTION action) { onCollisionAction = action; }
	inline void setDamage(std::string damage) { this->damage = damage; }
	inline void setAnimatable(Animatable animatable) { this->animatable = animatable; }
	inline void setLoopAnimation(bool loopAnimation) { this->loopAnimation = loopAnimation; }
	inline void setBaseSprite(Animatable baseSprite) { this->baseSprite = baseSprite; }
	inline void setHitboxRadius(std::string hitboxRadius) { this->hitboxRadius = hitboxRadius; }
	inline void setDespawnTime(float despawnTime) { this->despawnTime = despawnTime; }
	void setSpawnType(std::shared_ptr<EMPSpawnType> spawnType);
	void setSpawnTypeTime(std::string time);
	void setShadowTrailInterval(std::string shadowTrailInterval);
	void setShadowTrailLifespan(std::string shadowTrailLifespan);
	void setInheritRadius(bool inheritRadius, const LevelPack& levelPack);
	void setInheritDespawnTime(bool inheritDespawnTime, const LevelPack& levelPack);
	void setInheritShadowTrailInterval(bool inheritShadowTrailInterval, const LevelPack& levelPack);
	void setInheritShadowTrailLifespan(bool inheritShadowTrailLifespan, const LevelPack& levelPack);
	void setInheritAnimatables(bool inheritAnimatables, const LevelPack& levelPack);
	void setInheritDamage(bool inheritDamage, const LevelPack& levelPack);
	void setInheritPierceResetTime(bool inheritPierceResetTime, const LevelPack& levelPack);
	void setInheritSoundSettings(bool inheritSoundSettings, const LevelPack& levelPack);
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

	int getTreeSize() const;
	float searchLargestHitbox() const;
	/*
	Searches for the child of this subtree with the ID
	*/
	std::shared_ptr<EditorMovablePoint> searchID(int id) const;

	/*
	For testing.
	*/
	bool operator==(const EditorMovablePoint& other) const;

private:
	// ID is unique only to the attack and is not saved
	int id;

	// Points to the EMP ID generator in the EditorAttack this EMP is a child of
	IDGenerator* idGen;

	bool isBullet = true;

	// Radius of the EMP's hitbox
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(hitboxRadius, float, 0)

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
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(shadowTrailInterval, float, 0.15)
	// Set to 0 or a negative number to disable shadow trail
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(shadowTrailLifespan, float, 0)

	// Only applicable if the EMP is not a bullet (hitboxRadius <= 0)
	Animatable animatable;
	// Only applicable if animatable is an animation
	bool loopAnimation = true;
	// The animatable that will be used after the animation ends. Only necessary if animatable is an animation and loopAnimation is false
	Animatable baseSprite;

	// The amount of damage this bullet deals
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(damage, int, 1)

	// Only for bullets; determines what happens when the bullet makes contact with something
	BULLET_ON_COLLISION_ACTION onCollisionAction = BULLET_ON_COLLISION_ACTION::DESTROY_THIS_BULLET_ONLY;
	// Time after hitting an enemy that the entity is able to be hit by this same bullet again; only for PIERCE_ENTITY onCollisionAction
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(pierceResetTime, float, 2)
	
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
	bool inheritPierceResetTime = true;
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