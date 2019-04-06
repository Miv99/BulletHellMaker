#pragma once
#include <vector>
#include <utility>
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include "TimeFunctionVariable.h"
#include "Constants.h"
#include "Components.h"

/*
A point that can move.
MP for short.
*/
class MovablePoint {
public:
	/*
	returnGlobalPositions - if true, the return value of this MovablePoint should not be added to the position of its reference entity, if any.
							This is only here because of HomingMP, which uses weird black magic.
	*/
	inline MovablePoint(float lifespan, bool returnGlobalPositions) : lifespan(lifespan), returnGlobalPositions(returnGlobalPositions) {}
	/*
	Returns the global position of the MP at some time.
	0 <= time <= lifespan
	*/
	inline sf::Vector2f compute(sf::Vector2f relativeTo, float time) {
		if (returnGlobalPositions) {
			return evaluate(time);
		} else {
			return relativeTo + evaluate(time);
		}
	}

	inline float getLifespan() {
		return lifespan;
	}
	inline void setLifespan(float lifespan) {
		this->lifespan = lifespan;
	}

protected:
	// Lifespan of the MP in seconds
	// Only purpose is to make it known how long an MP SHOULD be alive; computing a position past an MP's lifespan should work
	float lifespan;
	bool returnGlobalPositions;

private:
	virtual sf::Vector2f evaluate(float time) = 0;
};

/*
MP that combines multiple MPs into a single MP.
The order of the MPs in the constructor matters.
*/
class AggregatorMP : public MovablePoint {
public:
	AggregatorMP(std::vector<std::shared_ptr<MovablePoint>> movablePoints) : MovablePoint(this->calculateLifespan(), false), mps(movablePoints) {
		calculateMinTimes();
	}

	inline void clearMPs() {
		mps.clear();
		minTimes.clear();
		calculateLifespan();
	}
	inline void pushBackMP(std::shared_ptr<MovablePoint> newPoint) {
		mps.push_back(newPoint);
		calculateMinTimes();
		calculateLifespan();
	}
	inline std::shared_ptr<MovablePoint> getLastMP() {
		return mps[mps.size() - 1];
	}

private:
	std::vector<std::shared_ptr<MovablePoint>> mps;
	// The minimum amount of time before reaching the MP; index of minTimes corresponds to index of mps
	std::vector<float> minTimes;

	// Should be called every time mps is updated
	void calculateMinTimes() {
		float curTime = 0.0f;
		minTimes.clear();
		for (std::shared_ptr<MovablePoint> mp : mps) {
			minTimes.push_back(curTime);
			curTime += mp->getLifespan();
		}
	}
	float calculateLifespan() {
		lifespan = 0;
		for (std::shared_ptr<MovablePoint> mp : mps) {
			lifespan += mp->getLifespan();
		}
		return lifespan;
	}
	sf::Vector2f evaluate(float time) override {
		for (int i = mps.size() - 1; i >= 0; i--) {
			if (time >= minTimes[i]) {
				return mps[i]->compute(sf::Vector2f(0, 0), time - minTimes[i]);
			}
		}
	}
};

/*
MP that always evaluates to an entity's position.
*/
class EntityMP : public MovablePoint {
public:
	EntityMP(const PositionComponent& entityPosition, float lifespan) : MovablePoint(lifespan, false), entityPosition(entityPosition) {}

private:
	const PositionComponent& entityPosition;

	inline sf::Vector2f evaluate(float time) override {
		return sf::Vector2f(entityPosition.getX(), entityPosition.getY());
	}
};

/*
MP that is stationary at a specific position.
Acts as the base unit of all concrete MovablePoint references.
*/
class StationaryMP : public MovablePoint {
public:
	inline StationaryMP(sf::Vector2f position, float lifespan) : MovablePoint(lifespan, false), position(position) {}

private:
	sf::Vector2f position;

	inline sf::Vector2f evaluate(float time) override {
		return position;
	}
};

/*
MP represented in polar coordinates.
*/
class PolarMP : public MovablePoint {
public:
	/*
	angle - in radians
	*/
	inline PolarMP(float lifespan, std::shared_ptr<TFV> distance, std::shared_ptr<TFV> angle) : MovablePoint(lifespan, false), angle(angle), distance(distance) {}

private:
	std::shared_ptr<TFV> angle;
	std::shared_ptr<TFV> distance;

	inline sf::Vector2f evaluate(float time) override {
		auto a = distance->evaluate(time);
		auto b = angle->evaluate(time);
		return sf::Vector2f(distance->evaluate(time) * cos(angle->evaluate(time)), distance->evaluate(time) * sin(angle->evaluate(time)));
	}
};

/*
MP represented by a Bezier curve.
*/
class BezierMP : public MovablePoint {
public:
	/*
	angle - in radians
	*/
	inline BezierMP(float lifespan, std::vector<sf::Vector2f> controlPoints) : MovablePoint(lifespan, false), controlPoints(controlPoints), numControlPoints(controlPoints.size()) {
		assert(controlPoints.size() <= 4 && "Maximum number of control points in a Bezier curve limited to 4 because of the computational power required for each evaluation.");
		assert(controlPoints.size() != 1 && "Number of control points must be greater than 1");
	}

private:
	const std::vector<sf::Vector2f> controlPoints;
	int numControlPoints;

	inline sf::Vector2f evaluate(float time) override {
		// Scale time to be in range [0, 1]
		time /= lifespan;
		
		if (numControlPoints == 2) {
			return controlPoints[0] + time * (controlPoints[1] - controlPoints[0]);
		} else if (numControlPoints == 3) {
			float a = 1 - time;
			return a*a*controlPoints[0] + 2.0f*a*time*controlPoints[1] + time*time*controlPoints[2];
		} else {
			float a = 1 - time;
			return (a*a*a*controlPoints[0]) + (3.0f * a*a*time*controlPoints[1]) + (3.0f * a*time*time*controlPoints[2]) + (time*time*time*controlPoints[3]);
		}
	}
};

/*
MP for homing in on a PositionComponent.

Warning: This MP is time-invariant (see CurrentAngleTFV for more details)
*/
class HomingMP : public MovablePoint {
public:
	/*
	homingStrength - in range (0, 1]
	from - the entity homing in on the tartet
	to - the target
	*/
	inline HomingMP(float lifespan, std::shared_ptr<TFV> speed, float homingStrength, uint32_t from, uint32_t to, entt::DefaultRegistry& registry) : MovablePoint(lifespan, true), speed(speed), homingStrength(homingStrength), registry(registry), from(from) {
		assert(homingStrength > 0 && homingStrength <= 1);
		angle = std::make_shared<CurrentAngleTFV>(registry, from, to);
		prevAngle = angle->evaluate(0);
	}

	sf::Vector2f evaluate(float time) override;

private:
	struct CachedPositionSearchComparator {
		int operator()(const std::pair<float, sf::Vector2f>& a, float b) {
			return a.first < b;
		}
	};
	
	struct CachedPositionComparator {
		int operator()(const std::pair<float, sf::Vector2f>& a, const std::pair<float, sf::Vector2f>& b) {
			return a.first < b.first;
		}
	};

	entt::DefaultRegistry& registry;
	uint32_t from;
	std::shared_ptr<TFV> angle;
	std::shared_ptr<TFV> speed;
	float homingStrength;

	float lastEvaluatedTime = 0;
	float prevAngle;

	// Vector of pairs of time and positions, sorted by ascending time
	std::vector<std::pair<float, sf::Vector2f>> cachedPositions;
};