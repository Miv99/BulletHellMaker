#pragma once
#include <cmath>
#include <string>
#include <memory>
#include <cassert>
#include <algorithm>
#include <utility>
#include "Constants.h"
#include "TextMarshallable.h"
#include "Components.h"
#include <entt/entt.hpp>
#include <utility>

/*
TimeFuncVar (TFV)
A variable that is a function of time.
Evaluation of a TFV does not change it internally, so the same TFV can be reused.
*/
class TFV : public TextMarshallable {
public:
	inline TFV() {}
	virtual std::shared_ptr<TFV> clone() = 0;

	virtual std::string format() const = 0;
	virtual void load(std::string formattedString) = 0;
	// Display name for the user
	virtual std::string getName() = 0;

	virtual float evaluate(float time) = 0;
};

/*
TFV whose value increases linearly with time.

float startValue
float endValue
float maxTime
*/
class LinearTFV : public TFV {
public:
	inline LinearTFV() {}
	inline LinearTFV(float startValue, float endValue, float maxTime) : startValue(startValue), endValue(endValue), maxTime(maxTime) {}
	std::shared_ptr<TFV> clone() override;

	std::string format() const override;
	void load(std::string formattedString) override;
	std::string getName() override { return "Linear"; }

	float evaluate(float time) {
		return startValue + (time / maxTime) * (endValue - startValue);
	}

	inline float getStartValue() { return startValue; }
	inline float getEndValue() { return endValue; }
	inline float getMaxTime() { return maxTime; }
	inline void setStartValue(float startValue) { this->startValue = startValue; }
	inline void setEndValue(float endValue) { this->endValue = endValue; }
	inline void setMaxTime(float maxTime) { this->maxTime = maxTime; }

private:
	float startValue;
	float endValue;
	float maxTime;
};

/*
TFV with a nonchanging value.

float value
*/
class ConstantTFV : public TFV {
public:
	inline ConstantTFV() {}
	inline ConstantTFV(float value) : value(value) {}
	std::shared_ptr<TFV> clone() override;

	std::string format() const override;
	void load(std::string formattedString) override;
	std::string getName() override { return "Constant"; }

	float evaluate(float time) {
		return value;
	}

private:
	float value;
};

/*
Sine wave TFV.
For cosine wave, add 1/2 period to phaseShift.

float period
float amplitude
float valueShift
float phaseShift = 0.0f
*/
class SineWaveTFV : public TFV {
public:
	inline SineWaveTFV() {}
	inline SineWaveTFV(float period, float amplitude, float valueShift, float phaseShift = 0.0f) : period(period), amplitude(amplitude), valueShift(valueShift), phaseShift(phaseShift) {}
	std::shared_ptr<TFV> clone() override;
	
	std::string format() const override;
	void load(std::string formattedString) override;
	std::string getName() override { return "Sine wave"; }
	
	inline float evaluate(float time) {
		return amplitude * (float)sin(time * PI2 / period + phaseShift) + valueShift;
	}

private:
	float period;
	float amplitude;
	float valueShift;
	float phaseShift;
};

/*
TFV whose value is distance travelled due to acceleration.

float initialDistance
float initialVelocity
float acceleration
*/
class ConstantAccelerationDistanceTFV : public TFV {
public:
	inline ConstantAccelerationDistanceTFV() {}
	inline ConstantAccelerationDistanceTFV(float initialDistance, float initialVelocity, float acceleration) : initialDistance(initialDistance), initialVelocity(initialVelocity), acceleration(acceleration) {}
	std::shared_ptr<TFV> clone() override;

	std::string format() const override;
	void load(std::string formattedString) override;
	std::string getName() override { return "Distance due to gravity"; }
	
	inline float evaluate(float time) override {
		return initialDistance + initialVelocity * time + 0.5f*acceleration*time*time;
	}

private:
	float initialDistance;
	float initialVelocity;
	float acceleration;
};

/*
TFV that starts slow and accelerates.

float startValue
float endValue
float maxTime
int dampeningFactor - range [1, infinity]; no upper limit but ~100 is already pretty high
*/
class DampenedStartTFV : public TFV {
public:
	inline DampenedStartTFV() {}
	inline DampenedStartTFV(float startValue, float endValue, float maxTime, int dampeningFactor) : dampeningFactor(dampeningFactor), startValue(startValue) {
		a = (endValue - startValue) / pow(maxTime, 0.08f*dampeningFactor + 1);
	}
	std::shared_ptr<TFV> clone() override;
	
	std::string format() const override;
	void load(std::string formattedString) override;
	std::string getName() override { return "Dampened start"; }
	
	inline float evaluate(float time) override {
		return a * pow(time, 0.08f*dampeningFactor + 1) + startValue;
	}

private:
	// Calculated value to scale graph correctly
	float a;
	float startValue;
	int dampeningFactor;
};

/*
TFV that starts fast and decelerates.

float startValue
float endValue
float maxTime
int dampeningFactor - range [1, infinity]; no upper limit but ~100 is already pretty high
*/
class DampenedEndTFV : public TFV {
public:
	inline DampenedEndTFV() {}
	inline DampenedEndTFV(float startValue, float endValue, float maxTime, int dampeningFactor) : dampeningFactor(dampeningFactor), endValue(endValue), maxTime(maxTime) {
		a = (endValue - startValue) / pow(maxTime, 0.08f*dampeningFactor + 1);
	}
	std::shared_ptr<TFV> clone() override;
	
	std::string format() const override;
	void load(std::string formattedString) override;
	std::string getName() override { return "Dampened end"; }
	
	inline float evaluate(float time) override {
		return -a * pow(maxTime - time, 0.08f*dampeningFactor + 1) + endValue;
	}

private:
	// Calculated value to scale graph correctly
	float a;
	float endValue;
	float maxTime;
	int dampeningFactor;
};

/*
TFV that starts slow and accelerates but decelerates towards the end.
Will reach the midpoint of the start and end value at half the max time.

float startValue
float endValue
float maxTime
int dampeningFactor - range [1, infinity]; no upper limit but ~100 is already pretty high
*/
class DoubleDampenedTFV : public TFV {
public:
	inline DoubleDampenedTFV() {}
	inline DoubleDampenedTFV(float startValue, float endValue, float maxTime, int dampeningFactor) : dampeningFactor(dampeningFactor), startValue(startValue), endValue(endValue), maxTime(maxTime) {
		a = 0.5f * (endValue - startValue) / pow(maxTime / 2.0f, 0.08f*dampeningFactor + 1);
	}
	std::shared_ptr<TFV> clone() override;
	
	std::string format() const override;
	void load(std::string formattedString) override;
	std::string getName() override { return "Dampened start and end"; }
	
	inline float evaluate(float time) override {
		if (time < maxTime / 2) {
			return a * pow(time, 0.08f*dampeningFactor + 1) + startValue;
		} else {
			return -a * pow(maxTime - time, 0.08f*dampeningFactor + 1) + endValue;
		}
	}

private:
	// Calculated value to scale graph correctly
	float a;
	float startValue;
	float endValue;
	float maxTime;
	int dampeningFactor;
};

/*
TFV that adds a constant value to the evaluation of the wrapped TFV.

std::shared_ptr<TFV> wrappedTFV
float valueTranslation
*/
class TranslationWrapperTFV : public TFV {
public:
	inline TranslationWrapperTFV() {}
	inline TranslationWrapperTFV(std::shared_ptr<TFV> wrappedTFV, float valueTranslation) : wrappedTFV(wrappedTFV), valueTranslation(valueTranslation) {
		assert(dynamic_cast<TranslationWrapperTFV*>(wrappedTFV.get()) == nullptr && "TranslationWrapperTFV should never wrap another; they can be combined");
	}
	std::shared_ptr<TFV> clone() override;

	std::string format() const override;
	void load(std::string formattedString) override;
	std::string getName() override { return "Translated"; }

	inline float evaluate(float time) override {
		return valueTranslation + wrappedTFV->evaluate(time);
	}

private:
	std::shared_ptr<TFV> wrappedTFV;
	float valueTranslation;
};

/*
TFV that always evaluates to the angle from one PositionComponent to another.

Warning: This TFV is time-invariant (meaning evaluate() ignores the time parameter, and the return depends on the positions of the two
PositionComponents when the function was called). Because of this time-invariance, any MovablePoint that uses this type of TFV will also
be time-invariant and thus must implement its own mechanism for being able to determine past positions (the future can never
be determined and this game does not require it).

Another warning: this TFV cannot be saved or loaded.
*/
class CurrentAngleTFV : public TFV {
public:
	/*
	from - the entity acting as the origin
	to - the entity being pointed to
	*/
	inline CurrentAngleTFV(entt::DefaultRegistry& registry, uint32_t from, uint32_t to) : registry(registry), from(from), to(to) {
		assert(registry.has<PositionComponent>(from) && registry.has<PositionComponent>(to) && registry.has<HitboxComponent>(to) && registry.has<HitboxComponent>(to));
	}
	std::shared_ptr<TFV> clone() override;

	inline std::string format() const override {
		assert(false && "CurrentAngleTFV cannot be saved.");
		return "";
	}
	inline void load(std::string formattedString) override {
		assert(false && "CurrentAngleTFV cannot be loaded. If it is ever used by something, that something must know that the TFV it is \
			using is a CurrentAngleTFV so that the CurrentAngleTFV can be constructed again.");
	}
	std::string getName() override { return "Angle to entity"; }

	float evaluate(float time) override {
		auto& fromPos = registry.get<PositionComponent>(from);
		auto& toPos = registry.get<PositionComponent>(to);
		auto& fromHitbox = registry.get<HitboxComponent>(from);
		auto& toHitbox = registry.get<HitboxComponent>(to);
		return std::atan2((toPos.getY() + toHitbox.getY()) - (fromPos.getY() + fromHitbox.getY()), (toPos.getX() + toHitbox.getX()) - (fromPos.getX() + fromHitbox.getX()));
	}

private:
	entt::DefaultRegistry& registry;
	uint32_t from;
	uint32_t to;
};

/*
TFV that connects multiple TFVs together in piecewise fashion.
Each TFV segment in PiecewiseTFV will be evaluated based on time since the start of that segment, not time since the start of PiecewiseTFV.
*/
class PiecewiseTFV : public TFV {
public:
	inline PiecewiseTFV() {}
	std::shared_ptr<TFV> clone() override;

	std::string format() const override;
	void load(std::string formattedString) override;
	std::string getName() override { return "Piecewise"; }

	inline float evaluate(float time) override {
		auto it = std::lower_bound(segments.begin(), segments.end(), time, SegmentComparator());
		if (it == segments.end()) {
			return segments.back().second->evaluate(time - segments.back().first);
		} else {
			return it->second->evaluate(time - it->first);
		}
	}

	/*
	Returns a pair containing, in order, the normal evaluation of the TFV and the index of the segment
	in which the evaluation occurred.
	*/
	inline std::pair<float, int> piecewiseEvaluate(float time) {
		auto it = std::lower_bound(segments.begin(), segments.end(), time, SegmentComparator());
		if (it == segments.end()) {
			return std::make_pair(segments.back().second->evaluate(time - segments.back().first), segments.size() - 1);
		} else {
			return std::make_pair(it->second->evaluate(time - it->first), it - segments.begin());
		}
	}

	/*
	segment - a pair with the float representing the lifespan of the TFV
	*/
	inline void insertSegment(int index, std::pair<float, std::shared_ptr<TFV>> segment) {
		// Recalculate active times
		for (int i = index; i < segments.size(); i++) {
			segments[i].first += segment.first;
		}
		if (index != 0) {
			segment.first += segments[index - 1].first;
		}
		segments.insert(segments.begin() + index, segment);
	}

	inline void removeSegment(int index) {
		auto erasedActiveTime = (segments.begin() + index)->first;
		segments.erase(segments.begin() + index);
		// Recalculate active times
		for (int i = index; i < segments.size(); i++) {
			segments[i].first -= erasedActiveTime;
		}
	}

	inline std::pair<float, std::shared_ptr<TFV>> getSegment(int index) {
		return segments[index];
	}

	inline int getSegmentsCount() {
		return segments.size();
	}

private:
	struct SegmentComparator {
		int operator()(const std::pair<float, std::shared_ptr<TFV>>& a, float b) {
			return a.first < b;
		}
	};

	// Vector of pairs of when the TFV becomes active and the TFV. The first item should become active at t=0.
	// Sorted in ascending order
	std::vector<std::pair<float, std::shared_ptr<TFV>>> segments;
};

class TFVFactory {
public:
	static std::shared_ptr<TFV> create(std::string formattedString);
};