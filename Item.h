#pragma once
#include <string>
#include <entt/entt.hpp>
#include "Components.h"
#include "Constants.h"
#include "TextMarshallable.h"
#include "Player.h"

/*
An item is something that can be picked up by the player and does something on pickup.
These classes act as templates and do not get modified, so they same Item can be used
for multiple item entities.
*/
class Item : public TextMarshallable {
public:
	inline Item() {}
	inline Item(Animatable animatable, float hitboxRadius, float activationRadius = 75.0f) : animatable(animatable), hitboxRadius(hitboxRadius), activationRadius(activationRadius) {}

	std::string format() = 0;
	void load(std::string formattedString) = 0;

	// Called when the player makes contact with an item's hitbox
	virtual void onPlayerContact(entt::DefaultRegistry& registry, uint32_t player) = 0;

	inline Animatable getAnimatable() { return animatable; }
	inline float getHitboxRadius() { return hitboxRadius; }
	inline float getActivationRadius() { return activationRadius; }

	inline void setAnimatable(Animatable animatable) { this->animatable = animatable; }
	inline void setHitboxRadius(float hitboxRadius) { this->hitboxRadius = hitboxRadius; }
	inline void setActivationRadius(float activationRadius) { this->activationRadius = activationRadius; }

protected:
	// If this is an animation, it always loops
	Animatable animatable;
	float hitboxRadius;
	// Minimum distance between an item and player before the item begins moving towards the player
	float activationRadius;
};

/*
Item that heals the player.
*/
class HealthPackItem : public Item {
public:
	inline HealthPackItem() : Item() {}
	inline HealthPackItem(Animatable animatable, float hitboxRadius, float activationRadius = 75.0f) : Item(animatable, hitboxRadius, activationRadius) {}

	std::string format() override;
	void load(std::string formattedString) override;

	inline void onPlayerContact(entt::DefaultRegistry& registry, uint32_t player) {
		registry.get<HealthComponent>(player).heal(HEALTH_PER_HEALTH_PACK);
	}
};

/*
Item that powers up the player.
*/
class PowerPackItem : public Item {
public:
	inline PowerPackItem() : Item() {}
	inline PowerPackItem(Animatable animatable, float hitboxRadius, float activationRadius = 75.0f) : Item(animatable, hitboxRadius, activationRadius) {}

	std::string format() override;
	void load(std::string formattedString) override;

	inline void onPlayerContact(entt::DefaultRegistry& registry, uint32_t player) {
		registry.get<PlayerTag>().increasePower(registry, player, POWER_PER_POWER_PACK);
	}
};

/*
Item that gives the player a bomb.
*/
class BombItem : public Item {
public:
	inline BombItem() : Item() {}
	inline BombItem(Animatable animatable, float hitboxRadius, float activationRadius = 75.0f) : Item(animatable, hitboxRadius, activationRadius) {}

	std::string format() override;
	void load(std::string formattedString) override;

	inline void onPlayerContact(entt::DefaultRegistry& registry, uint32_t player) {
		registry.get<PlayerTag>().gainBombs(registry, 1);
	}
};

/*
Item that adds to total points.
*/
class PointsPackItem : public Item {
public:
	inline PointsPackItem() : Item() {}
	inline PointsPackItem(Animatable animatable, float hitboxRadius, float activationRadius = 150.0f) : Item(animatable, hitboxRadius, activationRadius) {}

	std::string format() override;
	void load(std::string formattedString) override;

	inline void onPlayerContact(entt::DefaultRegistry& registry, uint32_t player) {
		registry.get<LevelManagerTag>().addPoints(POINTS_PER_POINTS_PACK);
	}
};

class ItemFactory {
public:
	static std::shared_ptr<Item> create(std::string formattedString);
};