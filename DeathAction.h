#pragma once
#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <entt/entt.hpp>
#include "TextMarshallable.h"
#include "EntityAnimatableSet.h"
#include "SpriteLoader.h"

/*
An action that is executed on the death of an entity.
Entity despawn does not count as death.
*/
class DeathAction : public TextMarshallable {
public:
	std::string format() = 0;
	void load(std::string formattedString) = 0;

	virtual void execute(entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, uint32_t entity) = 0;
};

class PlayAnimatableDeathAction : public DeathAction {
public:
	std::string format() override;
	void load(std::string formattedString) override;

	/*
	entity - the entity executing this DeathAction
	*/
	void execute(entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, uint32_t entity) override;

private:
	Animatable animatable;
	// Lifespan of the entity playing the death animation. Only applicable if animatable
	// is a sprite. Otherwise, the entity despawns when its animation is over.
	float duration;
};

class DeathActionFactory {
public:
	static std::shared_ptr<DeathAction> create(std::string formattedString);
};