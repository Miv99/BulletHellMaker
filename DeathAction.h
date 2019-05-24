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
#include "SpriteEffectAnimation.h"
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

/*
Death action for spawning some entity that displays an Animatable.
*/
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

	// Returns a reference
	inline Animatable& getAnimatable() { return animatable; }
	inline float getDuration() { return duration; }

	inline void setDuration(float duration) { this->duration = duration; }

private:
	Animatable animatable;
	// Type of sprite effect to apply to death animatable
	DEATH_ANIMATION_EFFECT effect = NONE;
	// Lifespan of the entity playing the death animation. Only applicable if animatable
	// is a sprite. Otherwise, the entity despawns when its animation is over.
	float duration;

	void loadEffectAnimation(SpriteComponent& sprite);
};

/*
Death action for playing a sound.
This is separate from a player/enemy's death sound.
*/
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

	// Returns a reference
	inline SoundSettings& getSoundSettings() { return soundSettings; }

private:
	SoundSettings soundSettings;
};

/*
Death action for executing attacks.
Note that this executes a number of attacks, not an attack pattern, meaning all EMPs will be spawned at t=0.
*/
class ExecuteAttacksDeathAction : public DeathAction {
public:
	inline ExecuteAttacksDeathAction() {}
	inline ExecuteAttacksDeathAction(std::vector<int> attackIDs) : attackIDs(attackIDs) {}

	std::string format() override;
	void load(std::string formattedString) override;

	void execute(LevelPack& levelPack, EntityCreationQueue& queue, entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, uint32_t entity) override;

	// Returns a reference
	inline std::vector<int>& getAttackIDs() { return attackIDs; }

private:
	std::vector<int> attackIDs;
};

// Used in ParticleExplosionDeathAction
static enum PARTICLE_EFFECT {
	NONE,
	FADE_AWAY,
	SHRINK
};
/*
Death action for an explosion of purely visual particles.
*/
class ParticleExplosionDeathAction : public DeathAction {
public:
	inline ParticleExplosionDeathAction() {}
	inline ParticleExplosionDeathAction(PARTICLE_EFFECT effect, Animatable animatable, bool loopAnimatable, sf::Color color, int minParticles = 20, int maxParticles = 30, float minDistance = 20, float maxDistance = 500, float minLifespan = 0.75f, float maxLifespan = 2.5f) : 
		effect(effect), animatable(animatable), color(color), minParticles(minParticles), maxParticles(maxParticles), minDistance(minDistance), maxDistance(maxDistance), minLifespan(minLifespan), maxLifespan(maxLifespan) {}

	std::string format() override;
	void load(std::string formattedString) override;

	void execute(LevelPack& levelPack, EntityCreationQueue& queue, entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, uint32_t entity) override;

	inline PARTICLE_EFFECT getEffect() { return effect; }
	// Returns a reference
	inline sf::Color& getColor() { return color; }

	inline void setEffect(PARTICLE_EFFECT effect) { this->effect = effect; }

private:
	PARTICLE_EFFECT effect;
	// Animatable used for the particle
	Animatable animatable;
	// Only applicable if animatable is an animation
	bool loopAnimatable;
	// Particle color. Note: this will overwrite the animatable's color as specified in its sprite sheet entry.
	sf::Color color = sf::Color::Yellow;
	// Number of particles
	int minParticles = 20, maxParticles = 30;
	// Min/max distance a particle can travel
	float minDistance = 20, maxDistance = 500;
	// Min/max lifespan of a particle
	float minLifespan = 0.75f, maxLifespan = 2.5f;
};

class DeathActionFactory {
public:
	static std::shared_ptr<DeathAction> create(std::string formattedString);
};