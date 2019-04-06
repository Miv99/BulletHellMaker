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

	virtual std::string format() = 0;
	virtual void load(std::string formattedString) = 0;

	virtual float evaluate(const entt::DefaultRegistry& registry, float xFrom, float yFrom) = 0;
};

/*
Angle in radians to the player.
*/
class EMPAAngleOffsetToPlayer : public EMPAAngleOffset {
public:
	inline EMPAAngleOffsetToPlayer() {}
	inline EMPAAngleOffsetToPlayer(float xOffset, float yOffset) : xOffset(xOffset), yOffset(yOffset) {}

	std::string format() override;
	void load(std::string formattedString) override;

	// Returns the angle in radians from coordinates (xFrom, yFrom) to the player plus the player offset (player.x + xOffset, player.y + yOffset)
	float evaluate(const entt::DefaultRegistry& registry, float xFrom, float yFrom) override;

private:
	float xOffset;
	float yOffset;
};

/*
Angle in radians to some global position.
*/
class EMPAAngleOffsetToGlobalPosition : public EMPAAngleOffset {
public:
	inline EMPAAngleOffsetToGlobalPosition() {}
	inline EMPAAngleOffsetToGlobalPosition(float x, float y) : x(x), y(y) {}

	std::string format() override;
	void load(std::string formattedString) override;

	// Returns the angle in radians from coordinates (xFrom, yFrom) to the global position (x, y)
	float evaluate(const entt::DefaultRegistry& registry, float xFrom, float yFrom) override;

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

	std::string format() override;
	void load(std::string formattedString) override;

	// Returns 0
	inline float evaluate(const entt::DefaultRegistry& registry, float xFrom, float yFrom) override { return 0; }

private:
	float x;
	float y;
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
	virtual std::string format() = 0;
	virtual void load(std::string formattedString) = 0;
	// Time for the action to be completed
	virtual float getTime() = 0;

	/*
	Generates a new MP from this EMPA.

	timeLag - the time elapsed since the generation was supposed to happen
	*/
	virtual std::shared_ptr<MovablePoint> execute(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float timeLag) = 0;
};

/*
EMPA for detaching from the parent (reference).
The new reference becomes a StationaryMP at the last known position.

Cannot be used by by enemies.
*/
class DetachFromParentEMPA : public EMPAction {
public:
	inline DetachFromParentEMPA() {}

	std::string format() override;
	void load(std::string formattedString) override;
	inline float getTime() override { return 0; }

	std::shared_ptr<MovablePoint> execute(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float timeLag) override;
};

/*
EMPA for when an MP stays still relative to its reference at the last position for some time.
*/
class StayStillAtLastPositionEMPA : public EMPAction {
public:
	inline StayStillAtLastPositionEMPA() {}
	inline StayStillAtLastPositionEMPA(float duration) : duration(duration) {}

	std::string format() override;
	void load(std::string formattedString) override;
	inline float getTime() override { return duration; }

	std::shared_ptr<MovablePoint> execute(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float timeLag) override;

private:
	float duration;
};

/*
EMPA for custom movement in polar coordinates.
*/
class MoveCustomPolarEMPA : public EMPAction {
public:
	inline MoveCustomPolarEMPA() {}
	inline MoveCustomPolarEMPA(std::shared_ptr<TFV> distance, std::shared_ptr<TFV> angle, float time) : distance(distance), angle(angle), time(time) {}
	inline MoveCustomPolarEMPA(std::shared_ptr<TFV> distance, std::shared_ptr<TFV> angle, float time, std::shared_ptr<EMPAAngleOffset> angleOffset) : distance(distance), angle(angle), time(time), angleOffset(angleOffset) {}

	std::string format() override;
	void load(std::string formattedString) override;
	inline float getTime() override { return time; }

	std::shared_ptr<MovablePoint> execute(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float timeLag) override;

private:
	std::shared_ptr<TFV> distance;
	std::shared_ptr<TFV> angle;
	// How long movement will last
	float time;
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
	inline MoveCustomBezierEMPA(std::vector<sf::Vector2f> controlPoints, float time) : controlPoints(controlPoints), time(time) {}

	std::string format() override;
	void load(std::string formattedString) override;
	inline float getTime() override { return time; }

	std::shared_ptr<MovablePoint> execute(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float timeLag) override;

private:
	std::vector<sf::Vector2f> controlPoints;
	// How long movement will last
	float time;
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
	inline MovePlayerHomingEMPA(float homingStrength, std::shared_ptr<TFV> speed, float time) : homingStrength(homingStrength), speed(speed), time(time) {}

	std::string format() override;
	void load(std::string formattedString) override;
	inline float getTime() override { return time; }

	std::shared_ptr<MovablePoint> execute(EntityCreationQueue& queue, entt::DefaultRegistry& registry, uint32_t entity, float timeLag) override;

private:
	// In range (0, 1]
	float homingStrength;
	// Speed at any instance in time
	std::shared_ptr<TFV> speed;
	// How long movement will last
	float time;
};

class EMPActionFactory {
public:
	static std::shared_ptr<EMPAction> create(std::string formattedString);
};