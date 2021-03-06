#pragma once
#include <string>
#include <vector>
#include <memory>

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

#include <DataStructs/TimeFunctionVariable.h>
#include <DataStructs/MovablePoint.h>
#include <Game/EntityCreationQueue.h>
#include <LevelPack/LevelPackObject.h>

struct EMPActionExecutionInfo {
	bool useNewReferenceEntity;
	uint32_t newReferenceEntity;

	std::shared_ptr<MovablePoint> newPath;
};

/*
Base class used to determine an angle in radians.
*/
class EMPAAngleOffset : public LevelPackObject {
public:
	EMPAAngleOffset();

	virtual std::shared_ptr<LevelPackObject> clone() const = 0;

	virtual std::string getName() = 0;

	virtual std::string format() const = 0;
	virtual void load(std::string formattedString) = 0;

	virtual nlohmann::json toJson() = 0;
	virtual void load(const nlohmann::json& j) = 0;

	virtual std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const = 0;
	virtual void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) = 0;

	virtual float evaluate(const entt::DefaultRegistry& registry, float xFrom, float yFrom) = 0;
	// Same as the other evaluate, but for when only the player's position is known
	virtual float evaluate(float xFrom, float yFrom, float playerX, float playerY) = 0;

	virtual bool operator==(const EMPAAngleOffset& other) const = 0;
};

/*
Angle in radians to the player.
*/
class EMPAAngleOffsetToPlayer : public EMPAAngleOffset {
public:
	EMPAAngleOffsetToPlayer(std::string xOffset = "0", std::string yOffset = "0");

	std::shared_ptr<LevelPackObject> clone() const override;

	inline std::string getName() override { return "Relative to player"; }

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	// Returns the angle in radians from coordinates (xFrom, yFrom) to the player plus the player offset (player.x + xOffset, player.y + yOffset)
	float evaluate(const entt::DefaultRegistry& registry, float xFrom, float yFrom) override;
	float evaluate(float xFrom, float yFrom, float playerX, float playerY) override;

	void setXOffset(std::string x) { xOffset = x; }
	void setYOffset(std::string y) { yOffset = y; }
	std::string getRawXOffset() { return xOffset; }
	std::string getRawYOffset() { return yOffset; }

	bool operator==(const EMPAAngleOffset& other) const override;

private:
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(xOffset, float, 0)
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(yOffset, float, 0)
};

/*
Angle in radians to some global position.
*/
class EMPAAngleOffsetToGlobalPosition : public EMPAAngleOffset {
public:
	EMPAAngleOffsetToGlobalPosition();
	EMPAAngleOffsetToGlobalPosition(std::string x, std::string y);

	std::shared_ptr<LevelPackObject> clone() const override;

	inline std::string getName() override { return "Absolute position"; }

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	// Returns the angle in radians from coordinates (xFrom, yFrom) to the global position (x, y)
	float evaluate(const entt::DefaultRegistry& registry, float xFrom, float yFrom) override;
	float evaluate(float xFrom, float yFrom, float playerX, float playerY) override;

	void setX(std::string x) { this->x = x; }
	void setY(std::string y) { this->y = y; }
	std::string getRawX() { return x; }
	std::string getRawY() { return y; }

	bool operator==(const EMPAAngleOffset& other) const override;

private:
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(x, float, 0)
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(y, float, 0)
};

/*
Angle offset that always returns 0.
*/
class EMPAAngleOffsetZero : public EMPAAngleOffset {
public:
	EMPAAngleOffsetZero();

	std::shared_ptr<LevelPackObject> clone() const override;

	inline std::string getName() override { return "No offset"; }

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	// Returns 0
	inline float evaluate(const entt::DefaultRegistry& registry, float xFrom, float yFrom) override { return 0; }
	inline float evaluate(float xFrom, float yFrom, float playerX, float playerY) override { return 0; }

	bool operator==(const EMPAAngleOffset& other) const override;
};

/*
Angle offset that always returns a constant, in radians.
*/
class EMPAAngleOffsetConstant : public EMPAAngleOffset {
public:
	EMPAAngleOffsetConstant();
	EMPAAngleOffsetConstant(std::string value);

	std::shared_ptr<LevelPackObject> clone() const override;

	inline std::string getName() override { return "Constant"; }

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	inline std::string getRawValue() const { return value; }
	inline void setValue(std::string value) { this->value = value; }

	inline float evaluate(const entt::DefaultRegistry& registry, float xFrom, float yFrom) override { return valueExprCompiledValue; }
	inline float evaluate(float xFrom, float yFrom, float playerX, float playerY) override { return valueExprCompiledValue; }

	bool operator==(const EMPAAngleOffset& other) const override;

private:
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(value, float, 0)
};

/*
Angle offset that always returns the angle that the player's sprite is facing.
*/
class EMPAngleOffsetPlayerSpriteAngle : public EMPAAngleOffset {
public:
	EMPAngleOffsetPlayerSpriteAngle();

	std::shared_ptr<LevelPackObject> clone() const override;

	inline std::string getName() override { return "Bind to player's direction"; }

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	float evaluate(const entt::DefaultRegistry& registry, float xFrom, float yFrom) override;
	float evaluate(float xFrom, float yFrom, float playerX, float playerY) override;

	bool operator==(const EMPAAngleOffset& other) const override;
};

class EMPAAngleOffsetFactory {
public:
	static std::shared_ptr<EMPAAngleOffset> create(std::string formattedString);
	static std::shared_ptr<EMPAAngleOffset> create(const nlohmann::json& j);
};
// -------------------------------------------------------------------------------------------------------------------------------------------

/*
Actions that EditorMovablePoints can do.
EMPA for short.
*/
class EMPAction : public LevelPackObject {
public:
	virtual std::shared_ptr<LevelPackObject> clone() const = 0;

	virtual std::string format() const = 0;
	virtual void load(std::string formattedString) = 0;

	virtual nlohmann::json toJson() = 0;
	virtual void load(const nlohmann::json& j) = 0;

	virtual std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const = 0;
	virtual void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) = 0;

	// Time for the action to be completed
	virtual float getTime() = 0;

	virtual void setTime(float duration) = 0;

	// String format when displayed in AttackEditor
	virtual std::string getGuiFormat() = 0;

	/*
	Generates a new MP from this EMPA and then changes the entity's reference entity to reflect the new MP.

	timeLag - the time elapsed since the generation was supposed to happen
	*/
	virtual std::shared_ptr<MovablePoint> execute(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float timeLag) = 0;
	/*
	Same thing as execute(), but no reference entity is created.

	x, y - the current position of whatever is going to use this MP
	*/
	virtual std::shared_ptr<MovablePoint> generateStandaloneMP(float x, float y, float playerX, float playerY) = 0;

	virtual bool operator==(const EMPAction& other) const = 0;
};

/*
EMPA for detaching from the parent (reference).
The new reference becomes a StationaryMP at the last known position.

Cannot be used by by enemies.
*/
class DetachFromParentEMPA : public EMPAction {
public:
	DetachFromParentEMPA();

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	inline float getTime() override { return 0; }
	std::string getGuiFormat() override;

	inline void setTime(float duration) override {}

	std::shared_ptr<MovablePoint> execute(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float timeLag) override;
	std::shared_ptr<MovablePoint> generateStandaloneMP(float x, float y, float playerX, float playerY) override;

	bool operator==(const EMPAction& other) const override;
};

/*
EMPA for when an MP stays still relative to its reference at the last position for some time.
*/
class StayStillAtLastPositionEMPA : public EMPAction {
public:
	StayStillAtLastPositionEMPA();
	StayStillAtLastPositionEMPA(float duration);

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	inline float getTime() override { return duration; }
	std::string getGuiFormat() override;

	inline void setTime(float duration) override { this->duration = duration; }

	std::shared_ptr<MovablePoint> execute(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float timeLag) override;
	std::shared_ptr<MovablePoint> generateStandaloneMP(float x, float y, float playerX, float playerY) override;

	bool operator==(const EMPAction& other) const override;

private:
	float duration = 0;
};

/*
EMPA for custom movement in polar coordinates.
*/
class MoveCustomPolarEMPA : public EMPAction {
public:
	MoveCustomPolarEMPA();
	MoveCustomPolarEMPA(std::shared_ptr<TFV> distance, std::shared_ptr<TFV> angle, float time);
	MoveCustomPolarEMPA(std::shared_ptr<TFV> distance, std::shared_ptr<TFV> angle, float time, std::shared_ptr<EMPAAngleOffset> angleOffset);
	
	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	std::string getGuiFormat() override;
	inline std::shared_ptr<TFV> getDistance() { return distance; }
	inline std::shared_ptr<TFV> getAngle() { return angle; }
	inline std::shared_ptr<EMPAAngleOffset> getAngleOffset() { return angleOffset; }
	inline float getTime() override { return time; }

	inline void setTime(float duration) override { this->time = duration; }
	inline void setDistance(std::shared_ptr<TFV> distance) { this->distance = distance; }
	inline void setAngle(std::shared_ptr<TFV> angle) { this->angle = angle; }
	inline void setAngleOffset(std::shared_ptr<EMPAAngleOffset> angleOffset) { this->angleOffset = angleOffset; }

	std::shared_ptr<MovablePoint> execute(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float timeLag) override;
	std::shared_ptr<MovablePoint> generateStandaloneMP(float x, float y, float playerX, float playerY) override;

	bool operator==(const EMPAction& other) const override;

private:
	std::shared_ptr<TFV> distance;
	std::shared_ptr<TFV> angle;
	// How long movement will last
	float time = 0;
	// Evaluates to the angle in radians that will be added to the angle TFV evaluation
	std::shared_ptr<EMPAAngleOffset> angleOffset;
};

/*
EMPA for custom movement with Bezier curves.
This type of EMPA can only be used by enemies.
The first control point must be at (0, 0) because all movement is done relative to the last known position of the entity.
*/
class MoveCustomBezierEMPA : public EMPAction {
public:
	MoveCustomBezierEMPA();
	MoveCustomBezierEMPA(std::vector<sf::Vector2f> unrotatedControlPoints, float time);
	MoveCustomBezierEMPA(std::vector<sf::Vector2f> unrotatedControlPoints, float time, std::shared_ptr<EMPAAngleOffset> rotationAngle);

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	inline float getTime() override { return time; }
	std::string getGuiFormat() override;
	inline std::shared_ptr<EMPAAngleOffset> getRotationAngle() { return rotationAngle; }
	const std::vector<sf::Vector2f> getUnrotatedControlPoints() { return unrotatedControlPoints; }

	inline void setTime(float duration) override { this->time = duration; }
	inline void setUnrotatedControlPoints(std::vector<sf::Vector2f> unrotatedControlPoints) { 
		this->unrotatedControlPoints = unrotatedControlPoints;
	}
	inline void setRotationAngle(std::shared_ptr<EMPAAngleOffset> rotationAngle) { this->rotationAngle = rotationAngle; }

	std::shared_ptr<MovablePoint> execute(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float timeLag) override;
	std::shared_ptr<MovablePoint> generateStandaloneMP(float x, float y, float playerX, float playerY) override;

	bool operator==(const EMPAction& other) const override;

private:
	// How long movement will last
	float time = 0;
	// Evaluates to the angle in radians that every control point will be rotated by (pivot at (0, 0))
	std::shared_ptr<EMPAAngleOffset> rotationAngle;
	// Only used if rotationAngle is not nullptr
	std::vector<sf::Vector2f> unrotatedControlPoints;
};

/*
EMPA for homing movement towards the player.
*/
class MovePlayerHomingEMPA : public EMPAction {
public:
	MovePlayerHomingEMPA();
	/*
	homingStrength - determines how quickly the entity homes in on the player; in range (0, 1]. A value of 0.02 is already pretty strong 
		and a value of 1.0 is a completely linear path assuming the player does not move
	*/
	MovePlayerHomingEMPA(std::shared_ptr<TFV> homingStrength, std::shared_ptr<TFV> speed, float time);

	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;

	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	std::string getGuiFormat() override;

	std::shared_ptr<MovablePoint> execute(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float timeLag) override;
	std::shared_ptr<MovablePoint> generateStandaloneMP(float x, float y, float playerX, float playerY) override;

	inline void setHomingStrength(std::shared_ptr<TFV> homingStrength) { this->homingStrength = homingStrength; }
	inline void setSpeed(std::shared_ptr<TFV> speed) { this->speed = speed; }
	inline void setTime(float time) override { this->time = time; }
	inline std::shared_ptr<TFV> getHomingStrength() { return homingStrength; }
	inline std::shared_ptr<TFV> getSpeed() { return speed; }
	inline float getTime() override { return time; }

	bool operator==(const EMPAction& other) const override;

private:
	// In range (0, 1]
	std::shared_ptr<TFV> homingStrength;
	// Speed at any instance in time
	std::shared_ptr<TFV> speed;
	// How long movement will last
	float time = 0;
};

/*
EMPA for homing movement towards a static global position.
*/
class MoveGlobalHomingEMPA : public EMPAction {
public:
	MoveGlobalHomingEMPA();
	/*
	homingStrength - determines how quickly the entity homes in on the player; in range (0, 1]. A value of 0.02 is already pretty strong
		and a value of 1.0 is a completely linear path assuming the player does not move
	*/
	MoveGlobalHomingEMPA(std::shared_ptr<TFV> homingStrength, std::shared_ptr<TFV> speed, std::string targetX, std::string targetY, float time);
	
	std::shared_ptr<LevelPackObject> clone() const override;

	std::string format() const override;
	void load(std::string formattedString) override;

	nlohmann::json toJson() override;
	void load(const nlohmann::json& j) override;
	
	std::pair<LEGAL_STATUS, std::vector<std::string>> legal(LevelPack& levelPack, SpriteLoader& spriteLoader, std::vector<exprtk::symbol_table<float>> symbolTables) const override;
	void compileExpressions(std::vector<exprtk::symbol_table<float>> symbolTables) override;

	std::string getGuiFormat() override;

	std::shared_ptr<MovablePoint> execute(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float timeLag) override;
	std::shared_ptr<MovablePoint> generateStandaloneMP(float x, float y, float playerX, float playerY) override;

	inline void setHomingStrength(std::shared_ptr<TFV> homingStrength) { this->homingStrength = homingStrength; }
	inline void setSpeed(std::shared_ptr<TFV> speed) { this->speed = speed; }
	inline void setTime(float time) override { this->time = time; }
	inline void setTargetX(std::string targetX) { this->targetX = targetX; }
	inline void setTargetY(std::string targetY) { this->targetY = targetY; }
	inline std::shared_ptr<TFV> getHomingStrength() { return homingStrength; }
	inline std::shared_ptr<TFV> getSpeed() { return speed; }
	inline float getTime() override { return time; }
	inline std::string getRawTargetX() const { return targetX; }
	inline std::string getRawTargetY() const { return targetY; }

	bool operator==(const EMPAction& other) const override;

private:
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(targetX, float, 0)
	DEFINE_EXPRESSION_VARIABLE_WITH_INITIAL_VALUE(targetY, float, 0)
	// In range (0, 1]
	std::shared_ptr<TFV> homingStrength;
	// Speed at any instance in time
	std::shared_ptr<TFV> speed;
	// How long movement will last
	float time = 0;
};

class EMPActionFactory {
public:
	static std::shared_ptr<EMPAction> create(std::string formattedString);
	static std::shared_ptr<EMPAction> create(const nlohmann::json& j);
};