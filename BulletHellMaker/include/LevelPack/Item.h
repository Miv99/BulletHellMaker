#pragma once
#include <string>

#include <entt/entt.hpp>

#include <Constants.h>
#include <LevelPack/TextMarshallable.h>
#include <LevelPack/Player.h>
#include <Game/AudioPlayer.h>
#include <LevelPack/ExpressionCompilable.h>
#include <LevelPack/LevelPackObject.h>

class LevelPack;

/*
An item is something that can be picked up by the player and does something on pickup.
*/
class Item : public LevelPackObject {
public:
	Item();
	Item(Animatable animatable, float hitboxRadius, float activationRadius = 75.0f);
	Item(Animatable animatable, float hitboxRadius, SoundSettings onCollectSound, float activationRadius = 75.0f);

	virtual std::shared_ptr<LevelPackObject> clone() const = 0;

	virtual std::string format() const = 0;
	virtual void load(std::string formattedString) = 0;

	virtual std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const = 0;
	virtual void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) = 0;

	// Called when the player makes contact with an item's hitbox
	virtual void onPlayerContact(entt::DefaultRegistry& registry, uint32_t player);

	inline Animatable getAnimatable() { return animatable; }
	inline float getHitboxRadius() { return hitboxRadius; }
	inline float getActivationRadius() { return activationRadius; }
	// Returns a reference
	inline SoundSettings& getOnCollectSound() { return onCollectSound; }

	inline void setAnimatable(Animatable animatable) { this->animatable = animatable; }
	inline void setHitboxRadius(float hitboxRadius) { this->hitboxRadius = hitboxRadius; }
	inline void setActivationRadius(float activationRadius) { this->activationRadius = activationRadius; }

protected:
	// If this is an animation, it always loops
	Animatable animatable;
	float hitboxRadius = 0;
	// Minimum distance between an item and player before the item begins moving towards the player
	float activationRadius = 0;

	// Sound to be played when item is collected by the player
	SoundSettings onCollectSound;
};

/*
Item that heals the player.
*/
class HealthPackItem : public Item {
public:
	HealthPackItem();
	HealthPackItem(Animatable animatable, float hitboxRadius, float activationRadius = 75.0f);
	HealthPackItem(Animatable animatable, float hitboxRadius, SoundSettings onCollectSound, float activationRadius = 75.0f);

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	void onPlayerContact(entt::DefaultRegistry& registry, uint32_t player);

private:
	// Health restored from this health pack
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(healthRestoreAmount, int, 1)
};

/*
Item that powers up the player.
*/
class PowerPackItem : public Item {
public:
	PowerPackItem();
	PowerPackItem(Animatable animatable, float hitboxRadius, float activationRadius = 75.0f);
	PowerPackItem(Animatable animatable, float hitboxRadius, SoundSettings onCollectSound, float activationRadius = 75.0f);

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	void onPlayerContact(entt::DefaultRegistry& registry, uint32_t player);

private:
	// Power received from this power pack
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(powerAmount, int, 1)
	// Number of points for every 1 power received after reaching the maximum
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(pointsPerExtraPower, int, 5)
};

/*
Item that gives the player a bomb.
*/
class BombItem : public Item {
public:
	BombItem();
	BombItem(Animatable animatable, float hitboxRadius, float activationRadius = 75.0f);
	BombItem(Animatable animatable, float hitboxRadius, SoundSettings onCollectSound, float activationRadius = 75.0f);

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	void onPlayerContact(entt::DefaultRegistry& registry, uint32_t player);

private:
	// Number of bombs received from this bomb item
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(bombsAmount, int, 1)
	// Number of points for every 1 bomb received after reaching the maximum
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(pointsPerExtraBomb, int, 1000)
};

/*
Item that adds to total points.
*/
class PointsPackItem : public Item {
public:
	PointsPackItem();
	PointsPackItem(Animatable animatable, float hitboxRadius, float activationRadius = 150.0f);
	PointsPackItem(Animatable animatable, float hitboxRadius, SoundSettings onCollectSound, float activationRadius = 150.0f);

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	void onPlayerContact(entt::DefaultRegistry& registry, uint32_t player);

private:
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(pointsAmount, int, 100)
};

class ItemFactory {
public:
	static std::shared_ptr<Item> create(std::string formattedString);
};