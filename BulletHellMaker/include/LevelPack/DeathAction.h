#pragma once
#include <string>
#include <vector>
#include <utility>
#include <memory>

#include <entt/entt.hpp>

#include <LevelPack/TextMarshallable.h>
#include <LevelPack/Animatable.h>
#include <DataStructs/SpriteLoader.h>
#include <Game/AudioPlayer.h>
#include <DataStructs/SpriteEffectAnimation.h>
#include <LevelPack/ExpressionCompilable.h>
#include <LevelPack/LevelPackObject.h>

class LevelPack;
class EntityCreationQueue;

/*
An action that is executed on the death of an entity.
Entity despawn does not count as death, unless the entity is a bullet.
*/
class DeathAction : public LevelPackObject {
public:
	virtual std::shared_ptr<LevelPackObject> clone() const = 0;

	virtual std::string format() const = 0;
	virtual void load(std::string formattedString) = 0;

	virtual nlohmann::json toJson() = 0;
	virtual void load(const nlohmann::json& j) = 0;

	virtual std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const = 0;
	virtual void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) = 0;

	/*
	entity - the entity executing this DeathAction
	*/
	virtual void execute(LevelPack& levelPack, EntityCreationQueue& queue, entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, uint32_t entity) = 0;
};

/*
Death action that does nothing.
*/
class NullDeathAction : public DeathAction {
public:
	NullDeathAction();

	std::shared_ptr<LevelPackObject> clone() const;

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables);

	void execute(LevelPack& levelPack, EntityCreationQueue& queue, entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, uint32_t entity) override;
};

/*
Death action for spawning some entity that displays an Animatable.
*/
class PlayAnimatableDeathAction : public DeathAction {
public:
	enum class DEATH_ANIMATION_EFFECT {
		NONE,
		// Animatable shrinks in size until it disappears
		SHRINK,
		// Animatable fades (alpha decreases) until it disappears
		FADE_AWAY
	};

	NLOHMANN_JSON_SERIALIZE_ENUM(DEATH_ANIMATION_EFFECT, {
		{DEATH_ANIMATION_EFFECT::NONE, "NONE"},
		{DEATH_ANIMATION_EFFECT::SHRINK, "SHRINK"},
		{DEATH_ANIMATION_EFFECT::FADE_AWAY, "FADE_AWAY"}
	})

	PlayAnimatableDeathAction();
	PlayAnimatableDeathAction(Animatable animatable, DEATH_ANIMATION_EFFECT effect, std::string duration);

	std::shared_ptr<LevelPackObject> clone() const;

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables);

	void execute(LevelPack& levelPack, EntityCreationQueue& queue, entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, uint32_t entity) override;

	Animatable getAnimatable();
	DEATH_ANIMATION_EFFECT getEffect();
	float getDuration();

	void setAnimatable(Animatable animatable);
	void setDuration(float duration);
	void setEffect(DEATH_ANIMATION_EFFECT effect);

private:
	Animatable animatable;
	// Type of sprite effect to apply to death animatable
	DEATH_ANIMATION_EFFECT effect = DEATH_ANIMATION_EFFECT::NONE;
	// Lifespan of the entity playing the death animation. Only applicable if animatable
	// is a sprite. Otherwise, the entity despawns when its animation is over.
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(duration, float, 0)
};

/*
Death action for playing a sound.
This is separate from a player/enemy's death sound.
*/
class PlaySoundDeathAction : public DeathAction {
public:
	PlaySoundDeathAction();
	/*
	fileName - file name with extension
	volume - volume multiplier in range [0, 100]
	*/
	PlaySoundDeathAction(SoundSettings soundSettings);

	std::shared_ptr<LevelPackObject> clone() const;

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables);

	void execute(LevelPack& levelPack, EntityCreationQueue& queue, entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, uint32_t entity) override;

	SoundSettings getSoundSettings();

	void setSoundSettings(SoundSettings soundSettings);

private:
	SoundSettings soundSettings;
};

/*
Death action for executing attacks.
Note that this executes a number of attacks, not an attack pattern, meaning all EMPs will be spawned at t=0.
*/
class ExecuteAttacksDeathAction : public DeathAction {
public:
	ExecuteAttacksDeathAction();
	ExecuteAttacksDeathAction(std::vector<std::pair<int, ExprSymbolTable>> attackIDs);

	std::shared_ptr<LevelPackObject> clone() const;

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables);

	void execute(LevelPack& levelPack, EntityCreationQueue& queue, entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, uint32_t entity) override;

	std::vector<std::pair<int, ExprSymbolTable>> getAttackIDs();

	void setAttackIDs(std::vector<std::pair<int, ExprSymbolTable>> attackIDs);

private:
	// Attack IDs and corresponding symbol definers
	std::vector<std::pair<int, ExprSymbolTable>> attackIDs;
	// This should be modified only internally. It will be populated after compileExpressions() is called. Any changes to attackIDs will
	// not be reflected here if compileExpressions() is not called anytime afterwards.
	std::vector<std::pair<int, exprtk::symbol_table<float>>> compiledAttackIDs;
};

/*
Death action for an explosion of purely visual particles.
*/
class ParticleExplosionDeathAction : public DeathAction {
public:
	enum class PARTICLE_EFFECT {
		NONE,
		// Particle fades in opacity
		FADE_AWAY,
		// Particle shrinks until it disappears
		SHRINK
	};

	NLOHMANN_JSON_SERIALIZE_ENUM(PARTICLE_EFFECT, {
		{PARTICLE_EFFECT::NONE, "NONE"},
		{PARTICLE_EFFECT::FADE_AWAY, "FADE_AWAY"},
		{PARTICLE_EFFECT::SHRINK, "SHRINK"}
	})

	ParticleExplosionDeathAction();
	ParticleExplosionDeathAction(PARTICLE_EFFECT effect, Animatable animatable, bool loopAnimatable, sf::Color color, std::string minParticles = "20", std::string maxParticles = "30",
		std::string minDistance = "20", std::string maxDistance = "500", std::string minLifespan = "0.75", std::string maxLifespan = "2.5");

	std::shared_ptr<LevelPackObject> clone() const;

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables);

	void execute(LevelPack& levelPack, EntityCreationQueue& queue, entt::DefaultRegistry& registry, SpriteLoader& spriteLoader, uint32_t entity) override;

	PARTICLE_EFFECT getEffect();
	sf::Color getColor();
	Animatable getAnimatable();
	bool getLoopAnimatable();
	int getMinParticles();
	int getMaxParticles();
	float getMinDistance();
	float getMaxDistance();
	float getMinLifespan();
	float getMaxLifespan();

	void setEffect(PARTICLE_EFFECT effect);
	void setColor(sf::Color color);
	void setAnimatable(Animatable animatable);
	void setLoopAnimatable(bool loopAnimatable);
	void setMinParticles(std::string minParticles);
	void setMaxParticles(std::string maxParticles);
	void setMinDistance(std::string minDistance);
	void setMaxDistance(std::string maxDistance);
	void setMinLifespan(std::string minLifespan);
	void setMaxLifespan(std::string maxLifespan);

private:
	PARTICLE_EFFECT effect = PARTICLE_EFFECT::FADE_AWAY;
	// Animatable used for the particle
	Animatable animatable;
	// Only applicable if animatable is an animation
	bool loopAnimatable = true;
	// Particle color. Note: this will overwrite the animatable's color as specified in its sprite sheet entry.
	sf::Color color = sf::Color::Red;
	// Number of particles
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(minParticles, int, 20)
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(maxParticles, int, 30)
	// Min/max distance a particle can travel
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(minDistance, float, 20)
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(maxDistance, float, 500)
	// Min/max lifespan of a particle in seconds
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(minLifespan, float, 0.75)
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(maxLifespan, float, 2.5)
};

class DeathActionFactory {
public:
	static std::shared_ptr<DeathAction> create(std::string formattedString);
	static std::shared_ptr<DeathAction> create(const nlohmann::json& j);
};