#pragma once
#include <cmath>
#include <string>
#include <memory>
#include <cassert>
#include "Constants.h"
#include "TextMarshallable.h"

/*
TimeFuncVar (TFV)
A variable that is a function of time.
Evaluation of a TFV does not change it internally, so the same TFV can be reused.
*/
class TFV : public TextMarshallable {
public:
	virtual std::string format() = 0;
	virtual void load(std::string formattedString) = 0;

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
	
	std::string format() override;
	void load(std::string formattedString) override;

	float evaluate(float time) {
		return startValue + (time / maxTime) * (endValue - startValue);
	}

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

	std::string format() override;
	void load(std::string formattedString) override;

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
	
	std::string format() override;
	void load(std::string formattedString) override;
	
	float evaluate(float time) {
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
	
	std::string format() override;
	void load(std::string formattedString) override;
	
	float evaluate(float time) override {
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
	
	std::string format() override;
	void load(std::string formattedString) override;
	
	float evaluate(float time) override {
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
	
	std::string format() override;
	void load(std::string formattedString) override;
	
	float evaluate(float time) override {
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
	
	std::string format() override;
	void load(std::string formattedString) override;
	
	float evaluate(float time) override {
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

	std::string format() override;
	void load(std::string formattedString) override;

	float evaluate(float time) override {
		return valueTranslation + wrappedTFV->evaluate(time);
	}

private:
	std::shared_ptr<TFV> wrappedTFV;
	float valueTranslation;
};

class TFVFactory {
public:
	static std::shared_ptr<TFV> create(std::string formattedString);
};