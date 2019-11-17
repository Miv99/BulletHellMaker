#pragma once
#include <string>
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include "TextMarshallable.h"
#include "TimeFunctionVariable.h"
#include "MovablePoint.h"
#include "EntityCreationQueue.h"

struct EMPActionExecutionInfo {
	bool useNewReferenceEntity;
	uint32_t newReferenceEntity;

	std::shared_ptr<MovablePoint> newPath;
};

/*
Angle in radians to some position.
*/
class EMPAAngleOffset : public TextMarshallable {
public:
	inline EMPAAngleOffset() {}
	virtual std::shared_ptr<EMPAAngleOffset> clone() = 0;

	virtual std::string format() const = 0;
	virtual void load(std::string formattedString) = 0;

	virtual float evaluate(const entt::DefaultRegistry& registry, float xFrom, float yFrom) = 0;
	// Same as the other evaluate, but for when only the player's position is known
	virtual float evaluate(float xFrom, float yFrom, float playerX, float playerY) = 0;
};

/*
Angle in radians to the player.
*/
class EMPAAngleOffsetToPlayer : public EMPAAngleOffset {
public:
	inline EMPAAngleOffsetToPlayer(float xOffset = 0, float yOffset = 0) : xOffset(xOffset), yOffset(yOffset) {}
	std::shared_ptr<EMPAAngleOffset> clone() override;

	std::string format() const override;
	void load(std::string formattedString) override;

	// Returns the angle in radians from coordinates (xFrom, yFrom) to the player plus the player offset (player.x + xOffset, player.y + yOffset)
	float evaluate(const entt::DefaultRegistry& registry, float xFrom, float yFrom) override;
	float evaluate(float xFrom, float yFrom, float playerX, float playerY) override;

private:
	float xOffset = 0;
	float yOffset = 0;
};

/*
Angle in radians to some global position.
*/
class EMPAAngleOffsetToGlobalPosition : public EMPAAngleOffset {
public:
	inline EMPAAngleOffsetToGlobalPosition() {}
	inline EMPAAngleOffsetToGlobalPosition(float x, float y) : x(x), y(y) {}
	std::shared_ptr<EMPAAngleOffset> clone() override;

	std::string format() const override;
	void load(std::string formattedString) override;

	// Returns the angle in radians from coordinates (xFrom, yFrom) to the global position (x, y)
	float evaluate(const entt::DefaultRegistry& registry, float xFrom, float yFrom) override;
	float evaluate(float xFrom, float yFrom, float playerX, float playerY) override;

private:
	float x;
	float y;
};

/*
Angle offset that always returns 0.
*/
class EMPAAngleOffsetZero : public EMPAAngleOffset {
public:
	inline EMPAAngleOffsetZero() {}
	std::shared_ptr<EMPAAngleOffset> clone() override;

	std::string format() const override;
	void load(std::string formattedString) override;

	// Returns 0
	inline float evaluate(const entt::DefaultRegistry& registry, float xFrom, float yFrom) override { return 0; }
	inline float evaluate(float xFrom, float yFrom, float playerX, float playerY) override { return 0; }
};

/*
Angle offset that always returns the angle that the player's sprite is facing.
*/
class EMPAngleOffsetPlayerSpriteAngle : public EMPAAngleOffset {
public:
	inline EMPAngleOffsetPlayerSpriteAngle() {}
	std::shared_ptr<EMPAAngleOffset> clone() override;

	std::string format() const override;
	void load(std::string formattedString) override;

	float evaluate(const entt::DefaultRegistry& registry, float xFrom, float yFrom) override;
	float evaluate(float xFrom, float yFrom, float playerX, float playerY) override;
};

class EMPAngleOffsetFactory {
public:
	static std::shared_ptr<EMPAAngleOffset> create(std::string formattedString);
};
// -------------------------------------------------------------------------------------------------------------------------------------------

/*
Actions that EditorMovablePoints can do.
EMPA for short.
*/
class EMPAction : public TextMarshallable {
public:
	virtual std::string format() const = 0;
	virtual void load(std::string formattedString) = 0;
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
};

/*
EMPA for detaching from the parent (reference).
The new reference becomes a StationaryMP at the last known position.

Cannot be used by by enemies.
*/
class DetachFromParentEMPA : public EMPAction {
public:
	inline DetachFromParentEMPA() {}

	std::string format() const override;
	void load(std::string formattedString) override;
	inline float getTime() override { return 0; }
	std::string getGuiFormat() override;

	inline void setTime(float duration) override {}

	std::shared_ptr<MovablePoint> execute(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float timeLag) override;
	std::shared_ptr<MovablePoint> generateStandaloneMP(float x, float y, float playerX, float playerY) override;
};

/*
EMPA for when an MP stays still relative to its reference at the last position for some time.
*/
class StayStillAtLastPositionEMPA : public EMPAction {
public:
	inline StayStillAtLastPositionEMPA() {}
	inline StayStillAtLastPositionEMPA(float duration) : duration(duration) {}

	std::string format() const override;
	void load(std::string formattedString) override;
	inline float getTime() override { return duration; }
	std::string getGuiFormat() override;

	inline void setTime(float duration) override { this->duration = duration; }

	std::shared_ptr<MovablePoint> execute(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float timeLag) override;
	std::shared_ptr<MovablePoint> generateStandaloneMP(float x, float y, float playerX, float playerY) override;

private:
	float duration = 0;
};

/*
EMPA for custom movement in polar coordinates.
*/
class MoveCustomPolarEMPA : public EMPAction {
public:
	inline MoveCustomPolarEMPA() {}
	inline MoveCustomPolarEMPA(std::shared_ptr<TFV> distance, std::shared_ptr<TFV> angle, float time) : distance(distance), angle(angle), time(time) {}
	inline MoveCustomPolarEMPA(std::shared_ptr<TFV> distance, std::shared_ptr<TFV> angle, float time, std::shared_ptr<EMPAAngleOffset> angleOffset) : distance(distance), angle(angle), time(time), angleOffset(angleOffset) {}

	std::string format() const override;
	void load(std::string formattedString) override;
	std::string getGuiFormat() override;
	inline std::shared_ptr<TFV> getDistance() { return distance; }
	inline std::shared_ptr<TFV> getAngle() { return angle; }

	inline float getTime() override { return time; }
	inline void setTime(float duration) override { this->time = duration; }
	inline void setDistance(std::shared_ptr<TFV> distance) { this->distance = distance; }
	inline void setAngle(std::shared_ptr<TFV> angle) { this->angle = angle; }

	std::shared_ptr<MovablePoint> execute(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float timeLag) override;
	std::shared_ptr<MovablePoint> generateStandaloneMP(float x, float y, float playerX, float playerY) override;

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
	inline MoveCustomBezierEMPA() {}
	inline MoveCustomBezierEMPA(std::vector<sf::Vector2f> unrotatedControlPoints, float time) : time(time) {
		setUnrotatedControlPoints(unrotatedControlPoints);
	}
	inline MoveCustomBezierEMPA(std::vector<sf::Vector2f> unrotatedControlPoints, float time, std::shared_ptr<EMPAAngleOffset> rotationAngle) : time(time), rotationAngle(rotationAngle) {
		setUnrotatedControlPoints(unrotatedControlPoints);
	}

	std::string format() const override;
	void load(std::string formattedString) override;
	inline float getTime() override { return time; }
	std::string getGuiFormat() override;

	inline void setTime(float duration) override { this->time = duration; }
	inline void setUnrotatedControlPoints(std::vector<sf::Vector2f> unrotatedControlPoints) { 
		this->unrotatedControlPoints = unrotatedControlPoints;
		controlPoints = std::vector<sf::Vector2f>(unrotatedControlPoints);
	}
	const std::vector<sf::Vector2f> getUnrotatedControlPoints() { return unrotatedControlPoints; }

	std::shared_ptr<MovablePoint> execute(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float timeLag) override;
	std::shared_ptr<MovablePoint> generateStandaloneMP(float x, float y, float playerX, float playerY) override;

private:
	// The first control point must be at (0, 0)
	std::vector<sf::Vector2f> controlPoints;
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
	inline MovePlayerHomingEMPA() {}
	/*
	homingStrength - determines how quickly the entity homes in on the player; in range (0, 1]. A value of 0.02 is already pretty strong.
	*/
	inline MovePlayerHomingEMPA(std::shared_ptr<TFV> homingStrength, std::shared_ptr<TFV> speed, float time) : homingStrength(homingStrength), speed(speed), time(time) {}

	std::string format() const override;
	void load(std::string formattedString) override;
	std::string getGuiFormat() override;

	std::shared_ptr<MovablePoint> execute(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float timeLag) override;
	std::shared_ptr<MovablePoint> generateStandaloneMP(float x, float y, float playerX, float playerY) override;

	inline void setHomingStrength(std::shared_ptr<TFV> homingStrength) { this->homingStrength = homingStrength; }
	inline void setSpeed(std::shared_ptr<TFV> speed) { this->speed = speed; }
	inline void setTime(float time) override { this->time = time; }
	inline std::shared_ptr<TFV> getHomingStrength() { return homingStrength; }
	inline std::shared_ptr<TFV> getSpeed() { return speed; }
	inline float getTime() override { return time; }

private:
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
};