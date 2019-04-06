#include "TimeFunctionVariable.h"

std::string ConstantTFV::format() {
	return "ConstantTFV" + delim + tos(value);
}

void ConstantTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	value = std::stof(items[1]);
}

std::string LinearTFV::format() {
	return "LinearTFV" + delim + tos(startValue) + delim + tos(endValue) + delim + tos(maxTime);
}

void LinearTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	startValue = std::stof(items[1]);
	endValue = std::stof(items[2]);
	maxTime = std::stof(items[3]);
}

std::string SineWaveTFV::format() {
	return "SineWaveTFV" + delim + tos(period) + delim + tos(amplitude) + delim + tos(valueShift) + delim + tos(phaseShift);
}

void SineWaveTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	period = std::stof(items[1]);
	amplitude = std::stof(items[2]);
	valueShift = std::stof(items[3]);
	phaseShift = std::stof(items[4]);
}

std::string ConstantAccelerationDistanceTFV::format() {
	return "ConstantAccelerationDistanceTFV" + delim + tos(initialDistance) + delim + tos(initialVelocity) + delim + tos(acceleration);
}

void ConstantAccelerationDistanceTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	initialDistance = std::stof(items[1]);
	initialVelocity = std::stof(items[2]);
	acceleration = std::stof(items[3]);
}

std::string DampenedStartTFV::format() {
	return "DampenedStartTFV" + delim + tos(a) + delim + tos(startValue) + delim + tos(dampeningFactor);
}

void DampenedStartTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	a = std::stof(items[1]);
	startValue = std::stof(items[2]);
	dampeningFactor = std::stoi(items[3]);
}

std::string DampenedEndTFV::format() {
	return "DampenedEndTFV" + delim + tos(a) + delim + tos(endValue) + delim + tos(maxTime) + delim + tos(dampeningFactor);
}

void DampenedEndTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	a = std::stof(items[1]);
	endValue = std::stof(items[2]);
	maxTime = std::stof(items[3]);
	dampeningFactor = std::stoi(items[4]);
}

std::string DoubleDampenedTFV::format() {
	return "DoubleDampenedTFV" + delim + tos(a) + delim + tos(startValue) + delim + tos(endValue) + delim + tos(maxTime) + delim + tos(dampeningFactor);
}

void DoubleDampenedTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	a = std::stof(items[1]);
	startValue = std::stof(items[2]);
	endValue = std::stof(items[3]);
	maxTime = std::stof(items[4]);
	dampeningFactor = std::stoi(items[5]);
}

std::string TranslationWrapperTFV::format() {
	return "TranslationWrapperTFV" + delim + tos(valueTranslation) + delim + "(" + wrappedTFV->format() + ")";
}

void TranslationWrapperTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	valueTranslation = std::stof(items[1]);
	wrappedTFV = TFVFactory::create(items[2]);
}

std::shared_ptr<TFV> TFVFactory::create(std::string formattedString) {
	auto name = split(formattedString, DELIMITER)[0];
	std::shared_ptr<TFV> ptr;
	if (name == "ConstantTFV") {
		ptr = std::make_shared<ConstantTFV>();
	}
	else if (name == "LinearTFV") {
		ptr = std::make_shared<LinearTFV>();
	}
	else if (name == "SineWaveTFV") {
		ptr = std::make_shared<SineWaveTFV>();
	}
	else if (name == "ConstantAccelerationDistanceTFV") {
		ptr = std::make_shared<ConstantAccelerationDistanceTFV>();
	}
	else if (name == "DampenedStartTFV") {
		ptr = std::make_shared<DampenedStartTFV>();
	}
	else if (name == "DampenedEndTFV") {
		ptr = std::make_shared<DampenedEndTFV>();
	}
	else if (name == "DoubleDampenedTFV") {
		ptr = std::make_shared<DoubleDampenedTFV>();
	}
	else if (name == "TranslationWrapperTFV") {
		ptr = std::make_shared<TranslationWrapperTFV>();
	}
	ptr->load(formattedString);
	return std::move(ptr);
}