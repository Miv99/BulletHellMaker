#pragma once
#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <entt/entt.hpp>
#include "TextMarshallable.h"
#include "Animatable.h"
#include "SpriteLoader.h"
#include "Components.h"
#include "AudioPlayer.h"
#include "EntityCreationQueue.h"

class LevelPack;

/*
An action that is executed on the death of an entity.
Entity despawn does not count as death, unless the entity is a bullet.
*/
class DeathAction : public TextMarshallable {
public:
	std::string format() = 0;
	void load(std::string formattedString) = 0;

	/*
	entity - the entity executing this DeathAction
	*/
	virtual void execute(LevelPack& levelPack, EntityCreationQueue& queue, entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, uint32_t entity) = 0;
};

class PlayAnimatableDeathAction : public DeathAction {
public:
	static enum DEATH_ANIMATION_EFFECT {
		NONE,
		// Animatable shrinks in size until it disappears
		SHRINK,
		// Animatable fades (alpha decreases) until it disappears
		FADE
	};

	inline PlayAnimatableDeathAction() {}
	inline PlayAnimatableDeathAction(Animatable animatable, DEATH_ANIMATION_EFFECT effect, float duration) : animatable(animatable), effect(effect), duration(duration) {}

	std::string format() override;
	void load(std::string formattedString) override;

	void execute(LevelPack& levelPack, EntityCreationQueue& queue, entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, uint32_t entity) override;

private:
	Animatable animatable;
	// Type of sprite effect to apply to death animatable
	DEATH_ANIMATION_EFFECT effect = NONE;
	// Lifespan of the entity playing the death animation. Only applicable if animatable
	// is a sprite. Otherwise, the entity despawns when its animation is over.
	float duration;

	void loadEffectAnimation(SpriteComponent& sprite);
};

class PlaySoundDeathAction : public DeathAction {
public:
	inline PlaySoundDeathAction() {}
	/*
	fileName - file name with extension
	volume - volume multiplier in range [0, 100]
	*/
	inline PlaySoundDeathAction(SoundSettings soundSettings) : soundSettings(soundSettings) {}

	std::string format() override;
	void load(std::string formattedString) override;

	void execute(LevelPack& levelPack, EntityCreationQueue& queue, entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, uint32_t entity) override;

private:
	SoundSettings soundSettings;
};

class DeathActionFactory {
public:
	static std::shared_ptr<DeathAction> create(std::string formattedString);
};