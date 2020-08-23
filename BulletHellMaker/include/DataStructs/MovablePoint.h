#pragma once
#include <vector>
#include <utility>

#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>

#include <Constants.h>
#include <Util/MathUtils.h>
#include <DataStructs/TimeFunctionVariable.h>
#include <Game/Components/PositionComponent.h>

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
	MovablePoint(float lifespan, bool returnGlobalPositions);

	/*
	Returns the global position of the MP at some time.
	0 <= time <= lifespan
	*/
	sf::Vector2f compute(sf::Vector2f relativeTo, float time);

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
	AggregatorMP(std::vector<std::shared_ptr<MovablePoint>> movablePoints);

	void clearMPs();
	void pushBackMP(std::shared_ptr<MovablePoint> newPoint);

	inline std::shared_ptr<MovablePoint> getLastMP() {
		return mps[mps.size() - 1];
	}

private:
	std::vector<std::shared_ptr<MovablePoint>> mps;
	// The minimum amount of time before reaching the MP; index of minTimes corresponds to index of mps
	std::vector<float> minTimes;

	/*
	Should be called every time mps is updated
	*/
	void calculateMinTimes();
	float calculateLifespan();
	sf::Vector2f evaluate(float time) override;
};

/*
MP that always evaluates to an entity's position.
*/
class EntityMP : public MovablePoint {
public:
	EntityMP(const PositionComponent& entityPosition, float lifespan);

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
	StationaryMP(sf::Vector2f position, float lifespan);

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
	PolarMP(float lifespan, std::shared_ptr<TFV> distance, std::shared_ptr<TFV> angle);

private:
	std::shared_ptr<TFV> angle;
	std::shared_ptr<TFV> distance;

	sf::Vector2f evaluate(float time);
};

/*
MP represented by a Bezier curve.
*/
class BezierMP : public MovablePoint {
public:
	/*
	angle - in radians
	*/
	BezierMP(float lifespan, std::vector<sf::Vector2f> controlPoints);

private:
	const std::vector<sf::Vector2f> controlPoints;
	int numControlPoints;

	sf::Vector2f evaluate(float time) override;
};

/*
MP for homing in on a PositionComponent or a global static position.

Warning: This MP is time-invariant (see CurrentAngleTFV for more details)
*/
class HomingMP : public MovablePoint {
public:
	/*
	homingStrength - in range (0, 1] always; a value of 0.02 is already pretty strong 
		and a value of 1.0 is a completely linear path assuming the target does not move
	from - the entity homing in on the target
	to - the target
	*/
	HomingMP(float lifespan, std::shared_ptr<TFV> speed, std::shared_ptr<TFV> homingStrength, uint32_t from, uint32_t to, entt::DefaultRegistry& registry);
	/*
	HomingMP that homes in on a static position.
	*/
	HomingMP(float lifespan, std::shared_ptr<TFV> speed, std::shared_ptr<TFV> homingStrength, uint32_t from, float toX, float toY, entt::DefaultRegistry& registry);
	/*
	This constructor is used just so EMPAs can generate standalone MPs without a registry.
	If a HomingMP is initialized with this constructor, evaluate() should never be called.
	*/
	HomingMP(float lifespan, std::shared_ptr<TFV> speed, std::shared_ptr<TFV> homingStrength, float fromX, float fromY, float toX, float toY);

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
	std::shared_ptr<TFV> homingStrength;

	float lastEvaluatedTime = 0;
	float prevAngle;

	// Vector of pairs of time and positions, sorted by ascending time
	std::vector<std::pair<float, sf::Vector2f>> cachedPositions;
};