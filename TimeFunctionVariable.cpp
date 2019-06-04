#include "TimeFunctionVariable.h"

std::string ConstantTFV::format() {
	return "ConstantTFV" + tm_delim + tos(value);
}

void ConstantTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	value = std::stof(items[1]);
}

std::string LinearTFV::format() {
	return "LinearTFV" + tm_delim + tos(startValue) + tm_delim + tos(endValue) + tm_delim + tos(maxTime);
}

void LinearTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	startValue = std::stof(items[1]);
	endValue = std::stof(items[2]);
	maxTime = std::stof(items[3]);
}

std::string SineWaveTFV::format() {
	return "SineWaveTFV" + tm_delim + tos(period) + tm_delim + tos(amplitude) + tm_delim + tos(valueShift) + tm_delim + tos(phaseShift);
}

void SineWaveTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	period = std::stof(items[1]);
	amplitude = std::stof(items[2]);
	valueShift = std::stof(items[3]);
	phaseShift = std::stof(items[4]);
}

std::string ConstantAccelerationDistanceTFV::format() {
	return "ConstantAccelerationDistanceTFV" + tm_delim + tos(initialDistance) + tm_delim + tos(initialVelocity) + tm_delim + tos(acceleration);
}

void ConstantAccelerationDistanceTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	initialDistance = std::stof(items[1]);
	initialVelocity = std::stof(items[2]);
	acceleration = std::stof(items[3]);
}

std::string DampenedStartTFV::format() {
	return "DampenedStartTFV" + tm_delim + tos(a) + tm_delim + tos(startValue) + tm_delim + tos(dampeningFactor);
}

void DampenedStartTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	a = std::stof(items[1]);
	startValue = std::stof(items[2]);
	dampeningFactor = std::stoi(items[3]);
}

std::string DampenedEndTFV::format() {
	return "DampenedEndTFV" + tm_delim + tos(a) + tm_delim + tos(endValue) + tm_delim + tos(maxTime) + tm_delim + tos(dampeningFactor);
}

void DampenedEndTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	a = std::stof(items[1]);
	endValue = std::stof(items[2]);
	maxTime = std::stof(items[3]);
	dampeningFactor = std::stoi(items[4]);
}

std::string DoubleDampenedTFV::format() {
	return "DoubleDampenedTFV" + tm_delim + tos(a) + tm_delim + tos(startValue) + tm_delim + tos(endValue) + tm_delim + tos(maxTime) + tm_delim + tos(dampeningFactor);
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
	return "TranslationWrapperTFV" + tm_delim + tos(valueTranslation) + tm_delim + "(" + wrappedTFV->format() + ")";
}

void TranslationWrapperTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	valueTranslation = std::stof(items[1]);
	wrappedTFV = TFVFactory::create(items[2]);
}

std::string PiecewiseContinuousTFV::format() {
	std::string ret = "";
	ret += "PiecewiseContinuousTFV";
	for (auto segment : segments) {
		ret += tm_delim + tos(segment.first) + tm_delim + "(" + segment.second->format() + ")";
	}
	return ret;
}

void PiecewiseContinuousTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	for (int i = 1; i < items.size(); i += 2) {
		segments.push_back(std::make_pair(std::stof(items[i]), TFVFactory::create(items[i + 1])));
	}
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
	else if (name == "PiecewiseContinuousTFV") {
		ptr = std::make_shared<PiecewiseContinuousTFV>();
	}
	ptr->load(formattedString);
	return std::move(ptr);
}