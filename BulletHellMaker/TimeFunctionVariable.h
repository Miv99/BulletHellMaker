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
#include <exception>

struct InvalidEvaluationDomainException : public std::exception {
	const char* what() const throw () {
		return "C++ Exception";
	}
};

/*
TimeFuncVar (TFV)
A variable that is a function of time.
Evaluation of a TFV does not change it internally, so the same TFV can be reused.
*/
class TFV : public TextMarshallable {
public:
	inline TFV() {}
	inline TFV(float maxTime) : maxTime(maxTime) {}
	virtual std::shared_ptr<TFV> clone() = 0;

	virtual std::string format() const = 0;
	virtual void load(std::string formattedString) = 0;
	// Display name for the user
	virtual std::string getName() = 0;
	
	inline float getMaxTime() { return maxTime; }
	virtual void setMaxTime(float maxTime) { this->maxTime = maxTime; }

	virtual float evaluate(float time) = 0;

	/*
	For testing.
	*/
	virtual bool operator==(const TFV& other) const = 0;

protected:
	// The lifespan of the TFV
	// Note that some types of TFVs don't need to know its own lifespan for evaluation
	float maxTime = 1;
};

/*
TFV whose value increases linearly with time.

float startValue
float endValue
float 
*/
class LinearTFV : public TFV {
public:
	inline LinearTFV() {}
	inline LinearTFV(float startValue, float endValue, float maxTime) : TFV(maxTime), startValue(startValue), endValue(endValue) {}
	std::shared_ptr<TFV> clone() override;

	std::string format() const override;
	void load(std::string formattedString) override;
	std::string getName() override { return "Linear"; }

	inline float evaluate(float time) {
		return startValue + (time / maxTime) * (endValue - startValue);
	}

	inline float getStartValue() { return startValue; }
	inline float getEndValue() { return endValue; }
	inline void setStartValue(float startValue) { this->startValue = startValue; }
	inline void setEndValue(float endValue) { this->endValue = endValue; }

	bool operator==(const TFV& other) const override;

private:
	float startValue = 0;
	float endValue = 0;
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

	inline float evaluate(float time) {
		return value;
	}

	inline void setValue(float value) { this->value = value; }
	inline float getValue() { return value; }

	bool operator==(const TFV& other) const override;

private:
	float value = 0;
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

	inline void setPeriod(float period) { this->period = period; }
	inline void setAmplitude(float amplitude) { this->amplitude = amplitude; }
	inline void setValueShift(float valueShift) { this->valueShift = valueShift; }
	inline void setPhaseShift(float phaseShift) { this->phaseShift = phaseShift; }
	inline float getPeriod() { return period; }
	inline float getAmplitude() { return amplitude; }
	inline float getValueShift() { return valueShift; }
	inline float getPhaseShift() { return phaseShift; }

	bool operator==(const TFV& other) const override;

private:
	float period = 1;
	float amplitude = 1;
	float valueShift = 0;
	float phaseShift = 0;
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
	std::string getName() override { return "Distance from acceleration"; }
	
	inline float evaluate(float time) override {
		return initialDistance + initialVelocity * time + 0.5f*acceleration*time*time;
	}

	inline void setInitialDistance(float initialDistance) { this->initialDistance = initialDistance; }
	inline void setInitialVelocity(float initialVelocity) { this->initialVelocity = initialVelocity; }
	inline void setAcceleration(float acceleration) { this->acceleration = acceleration; }
	inline float getInitialDistance() { return initialDistance; }
	inline float getInitialVelocity() { return initialVelocity; }
	inline float getAcceleration() { return acceleration; }

	bool operator==(const TFV& other) const override;

private:
	float initialDistance = 0;
	float initialVelocity = 0;
	float acceleration = 9.8f;
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
	inline DampenedStartTFV() {
		a = (endValue - startValue) / pow(maxTime, 0.08f*dampeningFactor + 1);
	}
	inline DampenedStartTFV(float startValue, float endValue, float maxTime, int dampeningFactor) : TFV(maxTime), dampeningFactor(dampeningFactor), startValue(startValue), endValue(endValue) {
		a = (endValue - startValue) / pow(maxTime, 0.08f*dampeningFactor + 1);
	}
	std::shared_ptr<TFV> clone() override;
	
	std::string format() const override;
	void load(std::string formattedString) override;
	std::string getName() override { return "Dampened start"; }
	
	inline float evaluate(float time) override {
		return a * pow(time, 0.08f*dampeningFactor + 1) + startValue;
	}

	inline void setStartValue(float startValue) {
		this->startValue = startValue;
		a = (endValue - startValue) / pow(maxTime, 0.08f*dampeningFactor + 1);
	}
	inline void setEndValue(float endValue) {
		this->endValue = endValue;
		a = (endValue - startValue) / pow(maxTime, 0.08f*dampeningFactor + 1);
	}
	inline void setDampeningFactor(int dampeningFactor) {
		this->dampeningFactor = dampeningFactor;
		a = (endValue - startValue) / pow(maxTime, 0.08f*dampeningFactor + 1);
	}
	inline void setMaxTime(float maxTime) override {
		this->maxTime = maxTime;
		a = 0.5f * (endValue - startValue) / pow(maxTime / 2.0f, 0.08f*dampeningFactor + 1);
	}
	inline float getStartValue() { return startValue; }
	inline float getEndValue() { return endValue; }
	inline int getDampeningFactor() { return dampeningFactor; }

	bool operator==(const TFV& other) const override;

private:
	// Calculated value to scale graph correctly
	float a;
	float startValue = 0;
	float endValue = 0;
	int dampeningFactor = 1;
};

/*
TFV that starts fast and decelerates.

float startValue
float endValue
int dampeningFactor - range [1, infinity]; no upper limit but ~100 is already pretty high
*/
class DampenedEndTFV : public TFV {
public:
	inline DampenedEndTFV() {
		a = (endValue - startValue) / pow(maxTime, 0.08f*dampeningFactor + 1);
	}
	inline DampenedEndTFV(float startValue, float endValue, float maxTime, int dampeningFactor) : TFV(maxTime), dampeningFactor(dampeningFactor), startValue(startValue), endValue(endValue) {
		a = (endValue - startValue) / pow(maxTime, 0.08f*dampeningFactor + 1);
	}
	std::shared_ptr<TFV> clone() override;
	
	std::string format() const override;
	void load(std::string formattedString) override;
	std::string getName() override { return "Dampened end"; }
	
	inline float evaluate(float time) override {
		return -a * pow(maxTime - time, 0.08f*dampeningFactor + 1) + endValue;
	}

	inline void setStartValue(float startValue) {
		this->startValue = startValue;
		a = (endValue - startValue) / pow(maxTime, 0.08f*dampeningFactor + 1);
	}
	inline void setEndValue(float endValue) {
		this->endValue = endValue;
		a = (endValue - startValue) / pow(maxTime, 0.08f*dampeningFactor + 1);
	}
	inline void setDampeningFactor(int dampeningFactor) {
		this->dampeningFactor = dampeningFactor;
		a = (endValue - startValue) / pow(maxTime, 0.08f*dampeningFactor + 1);
	}
	inline void setMaxTime(float maxTime) override {
		this->maxTime = maxTime;
		a = 0.5f * (endValue - startValue) / pow(maxTime / 2.0f, 0.08f*dampeningFactor + 1);
	}
	inline float getStartValue() { return startValue; }
	inline float getEndValue() { return endValue; }
	inline int getDampeningFactor() { return dampeningFactor; }

	bool operator==(const TFV& other) const override;

private:
	// Calculated value to scale graph correctly
	float a;
	float startValue = 0;
	float endValue = 0;
	int dampeningFactor = 1;
};

/*
TFV that starts slow and accelerates but decelerates towards the end.
Will reach the midpoint of the start and end value at half the max time.

float startValue
float endValue
int dampeningFactor - range [1, infinity]; no upper limit but ~100 is already pretty high
*/
class DoubleDampenedTFV : public TFV {
public:
	inline DoubleDampenedTFV() {
		a = 0.5f * (endValue - startValue) / pow(maxTime / 2.0f, 0.08f*dampeningFactor + 1);
	}
	inline DoubleDampenedTFV(float startValue, float endValue, float maxTime, int dampeningFactor) : TFV(maxTime), dampeningFactor(dampeningFactor), startValue(startValue), endValue(endValue) {
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

	inline void setStartValue(float startValue) {
		this->startValue = startValue;
		a = 0.5f * (endValue - startValue) / pow(maxTime / 2.0f, 0.08f*dampeningFactor + 1);
	}
	inline void setEndValue(float endValue) {
		this->endValue = endValue;
		a = 0.5f * (endValue - startValue) / pow(maxTime / 2.0f, 0.08f*dampeningFactor + 1);
	}
	inline void setDampeningFactor(int dampeningFactor) {
		this->dampeningFactor = dampeningFactor;
		a = 0.5f * (endValue - startValue) / pow(maxTime / 2.0f, 0.08f*dampeningFactor + 1);
	}
	inline void setMaxTime(float maxTime) override {
		this->maxTime = maxTime;
		a = 0.5f * (endValue - startValue) / pow(maxTime / 2.0f, 0.08f*dampeningFactor + 1);
	}
	inline float getStartValue() { return startValue; }
	inline float getEndValue() { return endValue; }
	inline int getDampeningFactor() { return dampeningFactor; }

	bool operator==(const TFV& other) const override;

private:
	// Calculated value to scale graph correctly
	float a;
	float startValue = 0;
	float endValue = 0;
	int dampeningFactor = 1;
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

	bool operator==(const TFV& other) const override;

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
		assert(registry.has<PositionComponent>(from) && registry.has<PositionComponent>(to) && registry.has<HitboxComponent>(from) && registry.has<HitboxComponent>(to));
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

	bool operator==(const TFV& other) const override;

private:
	entt::DefaultRegistry& registry;
	uint32_t from;
	uint32_t to;
};

/*
TFV that connects multiple TFVs together in piecewise fashion.
Each TFV segment in PiecewiseTFV will be evaluated based on time since the start of that segment, not time since the start of PiecewiseTFV.

Some functions here have an optional parameter totalLifespan, which if set to be the maximum lifespan of the entire PiecewiseTFV will recalculate the maxTime of every segment.
*/
class PiecewiseTFV : public TFV {
public:
	inline PiecewiseTFV() {}
	std::shared_ptr<TFV> clone() override;

	std::string format() const override;
	void load(std::string formattedString) override;
	std::string getName() override {
		if (segments.size() > 1) {
			return "Piecewise";
		} else if (segments.size() == 1) {
			return segments[0].second->getName();
		}
		return "None";
	}

	float evaluate(float time) override;

	/*
	Returns a pair containing, in order, the normal evaluation of the TFV and the index of the segment
	in which the evaluation occurred.

	Throws InvalidEvaluationDomainException if the first segment starts at some time later than the time parameter.
	*/
	std::pair<float, int> piecewiseEvaluate(float time);

	/*
	This function should only be used for testing purposes.
	This function also recalculates the maxTime of every segment.

	segment - a pair with the float representing the lifespan of the TFV
	*/
	void insertSegment(int index, std::pair<float, std::shared_ptr<TFV>> segment, float totalLifespan = -1);

	/*
	This function also recalculates the maxTime of every segment.
	Returns the index at which the segment was inserted.

	segment - a pair with the float representing the start time of the TFV
	*/
	int insertSegment(std::pair<float, std::shared_ptr<TFV>> segment, float totalLifespan = -1);

	/*
	This function also recalculates the maxTime of every segment, except the one that was removed.
	*/
	void removeSegment(int index, float totalLifespan = -1);

	/*
	This function also recalculates the maxTime of every segment.
	*/
	void changeSegment(int index, std::shared_ptr<TFV> newTFV, float totalLifespan = -1);

	/*
	Return the new index of the modified segment in the segment list.
	This function also recalculates the maxTime of every segment.
	*/
	int changeSegmentStartTime(int segmentIndex, float newStartTime, float totalLifespan = -1);

	std::pair<float, std::shared_ptr<TFV>> getSegment(int index);

	int getSegmentsCount();
	void setMaxTime(float maxTime) override;

	bool operator==(const TFV& other) const override;

private:
	// Vector of pairs of when the TFV becomes active and the TFV. The first item should become active at t=0.
	// Sorted in ascending order
	std::vector<std::pair<float, std::shared_ptr<TFV>>> segments;

	/*
	Recalculate the maxTimes of every segment

	totalLifespan - the total lifespan of this TFV
	*/
	void recalculateMaxTimes(float totalLifespan);
};

class TFVFactory {
public:
	static std::shared_ptr<TFV> create(std::string formattedString);
};