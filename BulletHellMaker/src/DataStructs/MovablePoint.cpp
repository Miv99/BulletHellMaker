#include <DataStructs/MovablePoint.h>

#include <iostream>

MovablePoint::MovablePoint(float lifespan, bool returnGlobalPositions) 
	: lifespan(lifespan), returnGlobalPositions(returnGlobalPositions) {
}

sf::Vector2f MovablePoint::compute(sf::Vector2f relativeTo, float time) {
	if (returnGlobalPositions) {
		return evaluate(time);
	} else {
		return relativeTo + evaluate(time);
	}
}

float lerpRadians(float start, float end, float amount) {
	float difference = std::abs(end - start);
	if (difference > PI) {
		// We need to add on to one of the values.
		if (end > start) {
			// We'll add it on to start...
			start += PI2;
		} else {
			// Add it on to end.
			end += PI2;
		}
	}

	// Interpolate it.
	float value = (start + ((end - start) * amount));

	// Wrap it..
	float rangeZero = PI2;

	if (value >= 0 && value <= PI2)
		return value;

	return std::fmod(value, rangeZero);
}

AggregatorMP::AggregatorMP(std::vector<std::shared_ptr<MovablePoint>> movablePoints) : MovablePoint(this->calculateLifespan(), false), mps(movablePoints) {
	calculateMinTimes();
}

void AggregatorMP::clearMPs() {
	mps.clear();
	minTimes.clear();
	calculateLifespan();
}
void AggregatorMP::pushBackMP(std::shared_ptr<MovablePoint> newPoint) {
	mps.push_back(newPoint);
	calculateMinTimes();
	calculateLifespan();
}

void AggregatorMP::calculateMinTimes() {
	float curTime = 0.0f;
	minTimes.clear();
	for (std::shared_ptr<MovablePoint> mp : mps) {
		minTimes.push_back(curTime);
		curTime += mp->getLifespan();
	}
}

float AggregatorMP::calculateLifespan() {
	lifespan = 0;
	for (std::shared_ptr<MovablePoint> mp : mps) {
		lifespan += mp->getLifespan();
	}
	return lifespan;
}

sf::Vector2f AggregatorMP::evaluate(float time) {
	for (int i = mps.size() - 1; i >= 0; i--) {
		if (time >= minTimes[i]) {
			return mps[i]->compute(sf::Vector2f(0, 0), time - minTimes[i]);
		}
	}
}

HomingMP::HomingMP(float lifespan, std::shared_ptr<TFV> speed, std::shared_ptr<TFV> homingStrength, uint32_t from, uint32_t to, entt::DefaultRegistry& registry) : MovablePoint(lifespan, true), speed(speed), homingStrength(homingStrength), registry(registry), from(from) {
	angle = std::make_shared<CurrentAngleTFV>(registry, from, to);
	prevAngle = angle->evaluate(0);
}

HomingMP::HomingMP(float lifespan, std::shared_ptr<TFV> speed, std::shared_ptr<TFV> homingStrength, uint32_t from, float toX, float toY, entt::DefaultRegistry& registry) : MovablePoint(lifespan, true), speed(speed), homingStrength(homingStrength), registry(registry), from(from) {
	angle = std::make_shared<CurrentAngleTFV>(registry, from, toX, toY);
	prevAngle = angle->evaluate(0);
}

HomingMP::HomingMP(float lifespan, std::shared_ptr<TFV> speed, std::shared_ptr<TFV> homingStrength, float fromX, float fromY, float toX, float toY) : MovablePoint(lifespan, true), speed(speed), homingStrength(homingStrength), registry(registry) {
	angle = std::make_shared<ConstantTFV>(std::atan2(toY - fromY, toX - fromX));
	prevAngle = angle->evaluate(0);
}

sf::Vector2f HomingMP::evaluate(float time) {
	// This breaks OOP but I can't think of any other way to do it.
	// We know this function will only be called for the purpose of determining a position in the present (which 
	// can be calculated easily) or in the past in order to account for time lag on various actions. Since this game limits the time
	// between physics updates to MAX_PHYSICS_DELTA_TIME, the time lag will never exceed MAX_PHYSICS_DELTA_TIME. Knowing this,
	// and since this MP is time-invariant but still needs to determine past positions, we can cache known positions in only the last
	// MAX_PHYSICS_DELTA_TIME seconds and discard anything older, since those older positions will never need to be known.
	// Linear interpolation can be used as a close estimate to calculate past positions that are not exactly at the moment of caching.
	// eg If time=1 and time=2's positions were cached, to find the position at time=1.3, linearly interpolate between the positions
	// at time=1 and time=2.
	float deltaTime = time - lastEvaluatedTime;
	assert(deltaTime <= MAX_PHYSICS_DELTA_TIME + 0.0001f);

	// Note: comparisons are done agianst MAX_PHYSICS_DELTA_TIME + 0.0001f to account for floating point inaccuracies

	if (!registry.has<PositionComponent>(from)) {
		// Sometimes the game randomly crashes from this and idk why so this is just a safety precaution
		// TODO: log this
		// BOOST_LOG_TRIVIAL(error) << "HomingMP failed to find PositionComponent of entity id " << from;
		return sf::Vector2f(0, 0);
	}

	auto& fromPos = registry.get<PositionComponent>(from);
	if (deltaTime > 0) {
		lastEvaluatedTime = time;

		// Remove positions older than MAX_PHYSICS_DELTA_TIME
		auto it = cachedPositions.begin();
		while (it != cachedPositions.end()) {
			if (time - it->first > MAX_PHYSICS_DELTA_TIME + 0.0001f) {
				it = cachedPositions.erase(it);
			} else {
				// cachedPositions is always sorted, so break loop as soon as something newer than MAX_PHYSICS_DELTA_TIME is encountered
				break;
			}
		}

		// Calculate new position
		float radians = lerpRadians(prevAngle, angle->evaluate(time), homingStrength->evaluate(time)); // angle is time-invariant so really anything can be passed in as the time parameter
		prevAngle = radians;
		float curSpeed = speed->evaluate(time);
		auto newPos = sf::Vector2f(fromPos.getX() + std::cos(radians) * curSpeed * deltaTime, fromPos.getY() + std::sin(radians) * curSpeed * deltaTime);

		// Add current position to list of cached positions
		cachedPositions.push_back(std::make_pair(time, newPos));

		return newPos;
	} else if (deltaTime < 0 && cachedPositions.size() > 0) {
		// This function call is a request for some past position, so check the cache

		if (cachedPositions.size() == 1) {
			// Only 1 cached position, so that's the closest possible we can get

			return cachedPositions[0].second;
		}

		auto it = std::lower_bound(cachedPositions.begin(), cachedPositions.end(), time, CachedPositionSearchComparator()) - 1;
		if ((*it).first == time) {
			// Perfect match for a cached time
			
			return (*it).second;
		} else {
			// The requested time is in-between two cached positions, so linearly interpolate to estimate where this MP was at that time

			auto t1 = (*it).first;
			auto pos1 = (*it).second;
			it++;
			auto t2 = (*it).first;
			auto pos2 = (*it).second;
			return sf::Vector2f((pos2.x - pos1.x)*((time - t1) / (t2 - t1)) + pos1.x, (pos2.y - pos1.y)*((time - t1) / (t2 - t1)) + pos1.y);
		}
	} else {
		// deltaTime is 0
		return sf::Vector2f(fromPos.getX(), fromPos.getY());
	}
}

EntityMP::EntityMP(const PositionComponent& entityPosition, float lifespan) 
	: MovablePoint(lifespan, false), entityPosition(entityPosition) {
}

StationaryMP::StationaryMP(sf::Vector2f position, float lifespan) 
	: MovablePoint(lifespan, false), position(position) {
}

PolarMP::PolarMP(float lifespan, std::shared_ptr<TFV> distance, std::shared_ptr<TFV> angle) 
	: MovablePoint(lifespan, false), angle(angle), distance(distance) {
}

sf::Vector2f PolarMP::evaluate(float time) {
	auto a = distance->evaluate(time);
	auto b = angle->evaluate(time);
	return sf::Vector2f(distance->evaluate(time) * cos(angle->evaluate(time)), distance->evaluate(time) * sin(angle->evaluate(time)));
}

BezierMP::BezierMP(float lifespan, std::vector<sf::Vector2f> controlPoints) 
	: MovablePoint(lifespan, false), controlPoints(controlPoints), numControlPoints(controlPoints.size()) {
}

sf::Vector2f BezierMP::evaluate(float time) {
	// Scale time to be in range [0, 1]
	time /= lifespan;

	if (numControlPoints == 2) {
		return controlPoints[0] + time * (controlPoints[1] - controlPoints[0]);
	} else if (numControlPoints == 3) {
		float a = 1 - time;
		return a * a * controlPoints[0] + 2.0f * a * time * controlPoints[1] + time * time * controlPoints[2];
	} else if (numControlPoints == 4) {
		float a = 1 - time;
		return (a * a * a * controlPoints[0]) + (3.0f * a * a * time * controlPoints[1]) + (3.0f * a * time * time * controlPoints[2]) + (time * time * time * controlPoints[3]);
	} else {
		sf::Vector2f sum(0, 0);
		for (int i = 0; i < numControlPoints; i++) {
			sum += binom(numControlPoints - 1, i) * std::pow(1.0f - time, numControlPoints - 1 - i) * std::pow(time, i) * controlPoints[i];
		}
		return sum;
	}
}