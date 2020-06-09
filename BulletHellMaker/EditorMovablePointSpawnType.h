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
	inline EMPSpawnType(float time, float x, float y) : time(time), x(x), y(y) {}

	std::string format() const = 0;
	void load(std::string formattedString) = 0;

	/*
	entity - the entity spawning the EMP
	*/
	virtual MPSpawnInformation getSpawnInfo(entt::DefaultRegistry& registry, uint32_t entity, float timeLag) = 0;
	/*
	Same as getSpawnInfo(), but useReferenceEntity in the returned MPSpawnInformation will always be false
	*/
	virtual MPSpawnInformation getForcedDetachmentSpawnInfo(entt::DefaultRegistry& registry, float timeLag) = 0;
	inline float getTime() const { return time; }
	inline float getX() const { return x; }
	inline float getY() const { return y; }

	inline void setTime(float time) { this->time = time; }
	inline void setX(float x) { this->x = x; }
	inline void setY(float y) { this->y = y; }

	/*
	For testing.
	*/
	bool operator==(const EMPSpawnType& other) const;

protected:
	// Time when this EMP is spawned with t=0 being the spawning of this EMP's reference
	// This is ignored and the EMP is spawned instantly if it is the main EMP of an Attack
	float time;

	// How the x/y coordinates are interpreted is up to the implementation
	float x;
	float y;
};

/*
Spawn type for spawning an EMP at some specific global position.
*/
class SpecificGlobalEMPSpawn : public EMPSpawnType {
public:
	inline SpecificGlobalEMPSpawn() {}
	inline SpecificGlobalEMPSpawn(float time, float x, float y) : EMPSpawnType(time, x, y) {}

	std::string format() const override;
	void load(std::string formattedString) override;

	MPSpawnInformation getSpawnInfo(entt::DefaultRegistry& registry, uint32_t entity, float timeLag) override;
	MPSpawnInformation getForcedDetachmentSpawnInfo(entt::DefaultRegistry& registry, float timeLag) override;
};

/*
Spawn type for spawning an EMP at some position relative to the hitbox origin of an entity.
*/
class EntityRelativeEMPSpawn : public EMPSpawnType {
public:
	inline EntityRelativeEMPSpawn() {}
	inline EntityRelativeEMPSpawn(float time, float x, float y) : EMPSpawnType(time, x, y) {}

	std::string format() const override;
	void load(std::string formattedString) override;

	/*
	entity - the entity that is being used as the reference
	*/
	MPSpawnInformation getSpawnInfo(entt::DefaultRegistry& registry, uint32_t entity, float timeLag) override;
	MPSpawnInformation getForcedDetachmentSpawnInfo(entt::DefaultRegistry& registry, float timeLag) override;
};

/*
Spawn type for spawning an EMP at some position relative to the hitbox origin of an entity and then setting the spawned
EMP's parent as the MP of the entity.
*/
class EntityAttachedEMPSpawn : public EMPSpawnType {
public:
	inline EntityAttachedEMPSpawn() {}
	inline EntityAttachedEMPSpawn(float time, float x, float y) : EMPSpawnType(time, x, y) {}

	std::string format() const override;
	void load(std::string formattedString) override;

	/*
	entity - the entity that the spawned EMP will be attached to
	*/
	MPSpawnInformation getSpawnInfo(entt::DefaultRegistry& registry, uint32_t entity, float timeLag) override;
	MPSpawnInformation getForcedDetachmentSpawnInfo(entt::DefaultRegistry& registry, float timeLag) override;
};

class EMPSpawnTypeFactory {
public:
	static std::shared_ptr<EMPSpawnType> create(std::string formattedString);
};