#pragma once
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <memory>
#include "MovablePoint.h"
#include "TextMarshallable.h"

/*
Provides information on the spawn location of a MP.
*/
struct MPSpawnInformation {
	bool useReferenceEntity;
	
	// The entity whose MovementPathComponent will be a reference
	uint32_t referenceEntity;
	// Global position
	sf::Vector2f position;
};

/*
Spawn type for an EMP.
*/
class EMPSpawnType : public TextMarshallable {
public:
	inline EMPSpawnType() {}
	inline EMPSpawnType(float time) : time(time) {}

	std::string format() = 0;
	void load(std::string formattedString) = 0;

	/*
	entity - the entity spawning the EMP
	*/
	virtual MPSpawnInformation getSpawnInfo(const entt::DefaultRegistry& registry, uint32_t entity) = 0;
	inline float getTime() { return time; }

protected:
	// Time when this EMP is spawned with t=0 being the spawning of this EMP's reference
	// This is ignored and the EMP is spawned instantly if it is the main EMP of an Attack
	float time;
};

/*
Spawn type for spawning an EMP at some specific global position.
*/
class SpecificGlobalEMPSpawn : public EMPSpawnType {
public:
	inline SpecificGlobalEMPSpawn() {}
	inline SpecificGlobalEMPSpawn(float time, float x, float y) : EMPSpawnType(time), x(x), y(y) {}

	std::string format() override;
	void load(std::string formattedString) override;

	MPSpawnInformation getSpawnInfo(const entt::DefaultRegistry& registry, uint32_t entity) override;

private:
	float x;
	float y;
};

/*
Spawn type for spawning an EMP at some position relative to the entity doing the attack.
*/
class EnemyRelativeEMPSpawn : public EMPSpawnType {
public:
	inline EnemyRelativeEMPSpawn() {}
	inline EnemyRelativeEMPSpawn(float time, float x, float y) : EMPSpawnType(time), x(x), y(y) {}

	std::string format() override;
	void load(std::string formattedString) override;

	MPSpawnInformation getSpawnInfo(const entt::DefaultRegistry& registry, uint32_t entity) override;

private:
	float x;
	float y;
};

/*
Spawn type for spawning an EMP at some position relative to the entity and then setting the spawned
EMP's parent as the MP of the entity.
*/
class EnemyAttachedEMPSpawn : public EMPSpawnType {
public:
	inline EnemyAttachedEMPSpawn() {}
	inline EnemyAttachedEMPSpawn(float time, float x, float y) : EMPSpawnType(time), x(x), y(y) {}

	std::string format() override;
	void load(std::string formattedString) override;

	MPSpawnInformation getSpawnInfo(const entt::DefaultRegistry& registry, uint32_t entity) override;

private:
	float x;
	float y;
};

class EMPSpawnTypeFactory {
public:
	static std::shared_ptr<EMPSpawnType> create(std::string formattedString);
};