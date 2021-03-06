#pragma once
#include <string>
#include <vector>
#include <memory>

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

#include <DataStructs/MovablePoint.h>
#include <LevelPack/TextMarshallable.h>
#include <LevelPack/LevelPackObject.h>

/*
Provides information on the spawn location of a MP.
*/
struct MPSpawnInformation {
	bool useReferenceEntity = false;
	
	// The entity whose MovementPathComponent will be a reference
	uint32_t referenceEntity;
	// Global position
	sf::Vector2f position = sf::Vector2f(0, 0);
};

/*
Spawn type for an EMP.
*/
class EMPSpawnType : public LevelPackObject {
public:
	EMPSpawnType();
	EMPSpawnType(std::string time, std::string x, std::string y);
	/*
	This constructor should only be used during gameplay to avoid having to compile expressions for temporary objects.
	*/
	EMPSpawnType(float time, float x, float y);

	virtual std::shared_ptr<LevelPackObject> clone() const = 0;

	virtual std::string format() const = 0;
	virtual void load(std::string formattedString) = 0;

	virtual nlohmann::json toJson();
	void load(const nlohmann::json& j) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	/*
	entity - the entity spawning the EMP
	*/
	virtual MPSpawnInformation getSpawnInfo(entt::DefaultRegistry& registry, uint32_t entity, float timeLag) = 0;
	/*
	Same as getSpawnInfo(), but useReferenceEntity in the returned MPSpawnInformation will always be false
	*/
	virtual MPSpawnInformation getForcedDetachmentSpawnInfo(entt::DefaultRegistry& registry, float timeLag) = 0;
	inline float getTime() const { return timeExprCompiledValue; }
	inline float getX() const { return xExprCompiledValue; }
	inline float getY() const { return yExprCompiledValue; }
	inline std::string getRawTime() const { return time; }
	inline std::string getRawX() const { return x; }
	inline std::string getRawY() const { return y; }

	inline void setTime(std::string time) { this->time = time; }
	inline void setX(std::string x) { this->x = x; }
	inline void setY(std::string y) { this->y = y; }

	/*
	For testing.
	*/
	bool operator==(const EMPSpawnType& other) const;

protected:
	// Time when this EMP is spawned with t=0 being the spawning of this EMP's reference
	// This is ignored and the EMP is spawned instantly if it is the main EMP of an EditorAttack
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(time, float, 0)

	// How the x/y coordinates are interpreted is up to the implementation
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(x, float, 0)
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(y, float, 0)
};

/*
Spawn type for spawning an EMP at some specific global position.
*/
class SpecificGlobalEMPSpawn : public EMPSpawnType {
public:
	SpecificGlobalEMPSpawn();
	SpecificGlobalEMPSpawn(std::string time, std::string x, std::string y);
	/*
	This constructor should only be used during gameplay to avoid having to compile expressions for temporary objects.
	*/
	SpecificGlobalEMPSpawn(float time, float x, float y);

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;

	MPSpawnInformation getSpawnInfo(entt::DefaultRegistry& registry, uint32_t entity, float timeLag) override;
	MPSpawnInformation getForcedDetachmentSpawnInfo(entt::DefaultRegistry& registry, float timeLag) override;
};

/*
Spawn type for spawning an EMP at some position relative to the hitbox origin of an entity.
*/
class EntityRelativeEMPSpawn : public EMPSpawnType {
public:
	EntityRelativeEMPSpawn();
	EntityRelativeEMPSpawn(std::string time, std::string x, std::string y);
	/*
	This constructor should only be used during gameplay to avoid having to compile expressions for temporary objects.
	*/
	EntityRelativeEMPSpawn(float time, float x, float y);

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;

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
	EntityAttachedEMPSpawn();
	EntityAttachedEMPSpawn(std::string time, std::string x, std::string y);
	/*
	This constructor should only be used during gameplay to avoid having to compile expressions for temporary objects.
	*/
	EntityAttachedEMPSpawn(float time, float x, float y);

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;

	/*
	entity - the entity that the spawned EMP will be attached to
	*/
	MPSpawnInformation getSpawnInfo(entt::DefaultRegistry& registry, uint32_t entity, float timeLag) override;
	MPSpawnInformation getForcedDetachmentSpawnInfo(entt::DefaultRegistry& registry, float timeLag) override;
};

class EMPSpawnTypeFactory {
public:
	static std::shared_ptr<EMPSpawnType> create(std::string formattedString);
	static std::shared_ptr<EMPSpawnType> create(const nlohmann::json& j);
};