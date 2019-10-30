#include "MovablePoint.h"
#include <iostream>
#include <boost/log/trivial.hpp>

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
		BOOST_LOG_TRIVIAL(error) << "HomingMP failed to find PositionComponent of entity id " << from;
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
