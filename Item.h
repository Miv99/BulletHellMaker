#pragma once
#include <string>
#include <entt/entt.hpp>
#include "Components.h"
#include "Constants.h"
#include "TextMarshallable.h"

/*
An item is something that can be picked up by the player and does something on pickup.
These classes act as templates and do not get modified, so they same Item can be used
for multiple item entities.
*/
class Item : public TextMarshallable {
public:
	inline Item() {}
	inline Item(Animatable animatable, float hitboxRadius) : animatable(animatable), hitboxRadius(hitboxRadius) {}

	std::string format() = 0;
	void load(std::string formattedString) = 0;

	// Called when the player makes contact with an item's hitbox
	virtual void onPlayerContact(entt::DefaultRegistry& registry, uint32_t player) = 0;

	inline Animatable getAnimatable() { return animatable; }
	inline float getHitboxRadius() { return hitboxRadius; }

	inline void setAnimatable(Animatable animatable) { this->animatable = animatable; }
	inline void setHitboxRadius(float hitboxRadius) { this->hitboxRadius = hitboxRadius; }

protected:
	// If this is an animation, it always loops
	Animatable animatable;
	float hitboxRadius;
};

/*
Item that heals the player.
*/
class HealthPackItem : public Item {
public:
	inline HealthPackItem() : Item() {}
	inline HealthPackItem(Animatable animatable, float hitboxRadius) : Item(animatable, hitboxRadius) {}

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
	inline PowerPackItem(Animatable animatable, float hitboxRadius) : Item(animatable, hitboxRadius) {}

	std::string format() override;
	void load(std::string formattedString) override;

	inline void onPlayerContact(entt::DefaultRegistry& registry, uint32_t player) {
		//TODO
	}
};

/*
Item that adds to total points.
*/
class PointsPackItem : public Item {
public:
	inline PointsPackItem() : Item() {}
	inline PointsPackItem(Animatable animatable, float hitboxRadius) : Item(animatable, hitboxRadius) {}

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