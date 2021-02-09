#include <DataStructs/TimeFunctionVariable.h>

#include <Game/Components/HitboxComponent.h>
#include <Game/Components/PositionComponent.h>

TFV::TFV() {
}

TFV::TFV(float maxTime) 
	: maxTime(maxTime) {
}

ConstantTFV::ConstantTFV() {
}

ConstantTFV::ConstantTFV(float value) 
	: value(value) {
}

std::shared_ptr<TFV> ConstantTFV::clone() {
	return std::make_shared<ConstantTFV>(value);
}

std::string ConstantTFV::format() const {
	return formatString("ConstantTFV") + tos(value);
}

void ConstantTFV::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	value = std::stof(items.at(1));
}

nlohmann::json ConstantTFV::toJson() {
	return {
		{"className", "ConstantTFV"},
		{"maxTime", maxTime},
		{"value", value}
	};
}

void ConstantTFV::load(const nlohmann::json& j) {
	j.at("maxTime").get_to(maxTime);
	j.at("value").get_to(value);
}

bool ConstantTFV::operator==(const TFV& other) const {
	const ConstantTFV& derived = dynamic_cast<const ConstantTFV&>(other);
	return value == derived.value;
}

LinearTFV::LinearTFV() {
}

LinearTFV::LinearTFV(float startValue, float endValue, float maxTime) 
	: TFV(maxTime), startValue(startValue), endValue(endValue) {
}

std::shared_ptr<TFV> LinearTFV::clone() {
	return std::make_shared<LinearTFV>(startValue, endValue, maxTime);
}

std::string LinearTFV::format() const {
	return formatString("LinearTFV") +  tos(startValue) +  tos(endValue) +  tos(maxTime);
}

void LinearTFV::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	startValue = std::stof(items.at(1));
	endValue = std::stof(items.at(2));
	maxTime = std::stof(items.at(3));
}

nlohmann::json LinearTFV::toJson() {
	return {
		{"className", "LinearTFV"},
		{"maxTime", maxTime},
		{"startValue", startValue},
		{"endValue", endValue}
	};
}

void LinearTFV::load(const nlohmann::json& j) {
	j.at("maxTime").get_to(maxTime);
	j.at("startValue").get_to(startValue);
	j.at("endValue").get_to(endValue);
}

bool LinearTFV::operator==(const TFV& other) const {
	const LinearTFV& derived = dynamic_cast<const LinearTFV&>(other);
	return startValue == derived.startValue && endValue == derived.endValue;
}

SineWaveTFV::SineWaveTFV() {
}

SineWaveTFV::SineWaveTFV(float period, float amplitude, float valueShift, float phaseShift)
	: period(period), amplitude(amplitude), valueShift(valueShift), phaseShift(phaseShift) {
}

std::shared_ptr<TFV> SineWaveTFV::clone() {
	return std::make_shared<SineWaveTFV>(period, amplitude, valueShift, phaseShift);
}

std::string SineWaveTFV::format() const {
	return formatString("SineWaveTFV") +  tos(period) +  tos(amplitude) +  tos(valueShift) +  tos(phaseShift);
}

void SineWaveTFV::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	period = std::stof(items.at(1));
	amplitude = std::stof(items.at(2));
	valueShift = std::stof(items.at(3));
	phaseShift = std::stof(items.at(4));
}

nlohmann::json SineWaveTFV::toJson() {
	return {
		{"className", "SineWaveTFV"},
		{"maxTime", maxTime},
		{"period", period},
		{"amplitude", amplitude},
		{"valueShift", valueShift},
		{"phaseShift", phaseShift},
	};
}

void SineWaveTFV::load(const nlohmann::json& j) {
	j.at("maxTime").get_to(maxTime);
	j.at("period").get_to(period);
	j.at("amplitude").get_to(amplitude);
	j.at("valueShift").get_to(valueShift);
	j.at("phaseShift").get_to(phaseShift);
}

bool SineWaveTFV::operator==(const TFV& other) const {
	const SineWaveTFV& derived = dynamic_cast<const SineWaveTFV&>(other);
	return period == derived.period && amplitude == derived.amplitude && valueShift == derived.valueShift && phaseShift == derived.phaseShift;
}

ConstantAccelerationDistanceTFV::ConstantAccelerationDistanceTFV() {
}

ConstantAccelerationDistanceTFV::ConstantAccelerationDistanceTFV(float initialDistance, float initialVelocity, float acceleration)
	: initialDistance(initialDistance), initialVelocity(initialVelocity), acceleration(acceleration) {
}

std::shared_ptr<TFV> ConstantAccelerationDistanceTFV::clone() {
	return std::make_shared<ConstantAccelerationDistanceTFV>(initialDistance, initialVelocity, acceleration);
}

std::string ConstantAccelerationDistanceTFV::format() const {
	return formatString("ConstantAccelerationDistanceTFV") + tos(initialDistance) + tos(initialVelocity) + tos(acceleration);
}

void ConstantAccelerationDistanceTFV::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	initialDistance = std::stof(items.at(1));
	initialVelocity = std::stof(items.at(2));
	acceleration = std::stof(items.at(3));
}

nlohmann::json ConstantAccelerationDistanceTFV::toJson() {
	return {
		{"className", "ConstantAccelerationDistanceTFV"},
		{"maxTime", maxTime},
		{"initialDistance", initialDistance},
		{"initialVelocity", initialVelocity},
		{"acceleration", acceleration}
	};
}

void ConstantAccelerationDistanceTFV::load(const nlohmann::json& j) {
	j.at("maxTime").get_to(maxTime);
	j.at("initialDistance").get_to(initialDistance);
	j.at("initialVelocity").get_to(initialVelocity);
	j.at("acceleration").get_to(acceleration);
}

bool ConstantAccelerationDistanceTFV::operator==(const TFV& other) const {
	const ConstantAccelerationDistanceTFV& derived = dynamic_cast<const ConstantAccelerationDistanceTFV&>(other);
	return initialDistance == derived.initialDistance && initialVelocity == derived.initialVelocity && acceleration == derived.acceleration;
}

DampenedStartTFV::DampenedStartTFV() {
	a = (endValue - startValue) / pow(maxTime, 0.08f * dampeningFactor + 1);
}

DampenedStartTFV::DampenedStartTFV(float startValue, float endValue, float maxTime, int dampeningFactor) 
	: TFV(maxTime), dampeningFactor(dampeningFactor), startValue(startValue), endValue(endValue) {
	a = (endValue - startValue) / pow(maxTime, 0.08f * dampeningFactor + 1);
}

std::shared_ptr<TFV> DampenedStartTFV::clone() {
	std::shared_ptr<TFV> copy = std::make_shared<DampenedStartTFV>();
	copy->load(format());
	return copy;
}

std::string DampenedStartTFV::format() const {
	return formatString("DampenedStartTFV") + tos(a) + tos(startValue) + tos(endValue) + tos(dampeningFactor);
}

void DampenedStartTFV::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	a = std::stof(items.at(1));
	startValue = std::stof(items.at(2));
	endValue = std::stof(items.at(3));
	dampeningFactor = std::stoi(items.at(4));
}

nlohmann::json DampenedStartTFV::toJson() {
	return {
		{"className", "DampenedStartTFV"},
		{"maxTime", maxTime},
		{"startValue", startValue},
		{"endValue", endValue},
		{"dampeningFactor", dampeningFactor}
	};
}

void DampenedStartTFV::load(const nlohmann::json& j) {
	j.at("maxTime").get_to(maxTime);
	j.at("startValue").get_to(startValue);
	j.at("endValue").get_to(endValue);
	j.at("dampeningFactor").get_to(dampeningFactor);

	a = (endValue - startValue) / pow(maxTime, 0.08f * dampeningFactor + 1);
}

bool DampenedStartTFV::operator==(const TFV& other) const {
	const DampenedStartTFV& derived = dynamic_cast<const DampenedStartTFV&>(other);
	return a == derived.a && startValue == derived.startValue && endValue == derived.endValue && dampeningFactor == derived.dampeningFactor;
}

DampenedEndTFV::DampenedEndTFV() {
	a = (endValue - startValue) / pow(maxTime, 0.08f * dampeningFactor + 1);
}
DampenedEndTFV::DampenedEndTFV(float startValue, float endValue, float maxTime, int dampeningFactor) 
	: TFV(maxTime), dampeningFactor(dampeningFactor), startValue(startValue), endValue(endValue) {
	a = (endValue - startValue) / pow(maxTime, 0.08f * dampeningFactor + 1);
}

std::shared_ptr<TFV> DampenedEndTFV::clone() {
	std::shared_ptr<TFV> copy = std::make_shared<DampenedEndTFV>();
	copy->load(format());
	return copy;
}

std::string DampenedEndTFV::format() const {
	return formatString("DampenedEndTFV") + tos(a) + tos(startValue) + tos(endValue) + tos(maxTime) + tos(dampeningFactor);
}

void DampenedEndTFV::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	a = std::stof(items.at(1));
	startValue = std::stof(items.at(2));
	endValue = std::stof(items.at(3));
	maxTime = std::stof(items.at(4));
	dampeningFactor = std::stoi(items.at(5));
}

nlohmann::json DampenedEndTFV::toJson() {
	return {
		{"className", "DampenedEndTFV"},
		{"maxTime", maxTime},
		{"startValue", startValue},
		{"endValue", endValue},
		{"dampeningFactor", dampeningFactor}
	};
}

void DampenedEndTFV::load(const nlohmann::json& j) {
	j.at("maxTime").get_to(maxTime);
	j.at("startValue").get_to(startValue);
	j.at("endValue").get_to(endValue);
	j.at("dampeningFactor").get_to(dampeningFactor);

	a = (endValue - startValue) / pow(maxTime, 0.08f * dampeningFactor + 1);
}

void DampenedEndTFV::setStartValue(float startValue) {
	this->startValue = startValue;
	a = (endValue - startValue) / pow(maxTime, 0.08f * dampeningFactor + 1);
}

void DampenedEndTFV::setEndValue(float endValue) {
	this->endValue = endValue;
	a = (endValue - startValue) / pow(maxTime, 0.08f * dampeningFactor + 1);
}

void DampenedEndTFV::setDampeningFactor(int dampeningFactor) {
	this->dampeningFactor = dampeningFactor;
	a = (endValue - startValue) / pow(maxTime, 0.08f * dampeningFactor + 1);
}

void DampenedEndTFV::setMaxTime(float maxTime) {
	this->maxTime = maxTime;
	a = 0.5f * (endValue - startValue) / pow(maxTime / 2.0f, 0.08f * dampeningFactor + 1);
}

bool DampenedEndTFV::operator==(const TFV& other) const {
	const DampenedEndTFV& derived = dynamic_cast<const DampenedEndTFV&>(other);
	return a == derived.a && startValue == derived.startValue && endValue == derived.endValue && dampeningFactor == derived.dampeningFactor;
}

DoubleDampenedTFV::DoubleDampenedTFV() {
	a = 0.5f * (endValue - startValue) / pow(maxTime / 2.0f, 0.08f * dampeningFactor + 1);
}
DoubleDampenedTFV::DoubleDampenedTFV(float startValue, float endValue, float maxTime, int dampeningFactor) 
	: TFV(maxTime), dampeningFactor(dampeningFactor), startValue(startValue), endValue(endValue) {
	a = 0.5f * (endValue - startValue) / pow(maxTime / 2.0f, 0.08f * dampeningFactor + 1);
}

std::shared_ptr<TFV> DoubleDampenedTFV::clone() {
	return std::make_shared<DoubleDampenedTFV>(startValue, endValue, maxTime, dampeningFactor);
}

std::string DoubleDampenedTFV::format() const {
	return formatString("DoubleDampenedTFV") + tos(a) + tos(startValue) + tos(endValue) + tos(maxTime) + tos(dampeningFactor);
}

void DoubleDampenedTFV::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	a = std::stof(items.at(1));
	startValue = std::stof(items.at(2));
	endValue = std::stof(items.at(3));
	maxTime = std::stof(items.at(4));
	dampeningFactor = std::stoi(items.at(5));
}

nlohmann::json DoubleDampenedTFV::toJson() {
	return {
		{"className", "DoubleDampenedTFV"},
		{"maxTime", maxTime},
		{"startValue", startValue},
		{"endValue", endValue},
		{"dampeningFactor", dampeningFactor}
	};
}

void DoubleDampenedTFV::load(const nlohmann::json& j) {
	j.at("maxTime").get_to(maxTime);
	j.at("startValue").get_to(startValue);
	j.at("endValue").get_to(endValue);
	j.at("dampeningFactor").get_to(dampeningFactor);

	a = 0.5f * (endValue - startValue) / pow(maxTime / 2.0f, 0.08f * dampeningFactor + 1);
}

float DoubleDampenedTFV::evaluate(float time) {
	if (time < maxTime / 2) {
		return a * pow(time, 0.08f * dampeningFactor + 1) + startValue;
	} else {
		return -a * pow(maxTime - time, 0.08f * dampeningFactor + 1) + endValue;
	}
}

void DoubleDampenedTFV::setStartValue(float startValue) {
	this->startValue = startValue;
	a = 0.5f * (endValue - startValue) / pow(maxTime / 2.0f, 0.08f * dampeningFactor + 1);
}

void DoubleDampenedTFV::setEndValue(float endValue) {
	this->endValue = endValue;
	a = 0.5f * (endValue - startValue) / pow(maxTime / 2.0f, 0.08f * dampeningFactor + 1);
}

void DoubleDampenedTFV::setDampeningFactor(int dampeningFactor) {
	this->dampeningFactor = dampeningFactor;
	a = 0.5f * (endValue - startValue) / pow(maxTime / 2.0f, 0.08f * dampeningFactor + 1);
}

void DoubleDampenedTFV::setMaxTime(float maxTime) {
	this->maxTime = maxTime;
	a = 0.5f * (endValue - startValue) / pow(maxTime / 2.0f, 0.08f * dampeningFactor + 1);
}

bool DoubleDampenedTFV::operator==(const TFV& other) const {
	const DoubleDampenedTFV& derived = dynamic_cast<const DoubleDampenedTFV&>(other);
	return a == derived.a && startValue == derived.startValue && endValue == derived.endValue && dampeningFactor == derived.dampeningFactor;
}

TranslationWrapperTFV::TranslationWrapperTFV() {
}

TranslationWrapperTFV::TranslationWrapperTFV(std::shared_ptr<TFV> wrappedTFV, float valueTranslation) 
	: wrappedTFV(wrappedTFV), valueTranslation(valueTranslation) {
}

std::shared_ptr<TFV> TranslationWrapperTFV::clone() {
	std::shared_ptr<TFV> copy = std::make_shared<TranslationWrapperTFV>();
	copy->load(format());
	return copy;
}

std::string TranslationWrapperTFV::format() const {
	return formatString("TranslationWrapperTFV") +  tos(valueTranslation) +  "(" + wrappedTFV->format() + ")";
}

void TranslationWrapperTFV::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	valueTranslation = std::stof(items.at(1));
	wrappedTFV = TFVFactory::create(items.at(2));
}

nlohmann::json TranslationWrapperTFV::toJson() {
	return {
		{"className", "TranslationWrapperTFV"},
		{"maxTime", maxTime},
		{"wrappedTFV", wrappedTFV->toJson()},
		{"valueTranslation", valueTranslation}
	};
}

void TranslationWrapperTFV::load(const nlohmann::json& j) {
	j.at("maxTime").get_to(maxTime);
	
	if (j.contains("wrappedTFV")) {
		wrappedTFV = TFVFactory::create(j.at("wrappedTFV"));
	} else {
		wrappedTFV = std::make_shared<ConstantTFV>(0);
	}

	j.at("valueTranslation").get_to(valueTranslation);
}

bool TranslationWrapperTFV::operator==(const TFV& other) const {
	const TranslationWrapperTFV& derived = dynamic_cast<const TranslationWrapperTFV&>(other);
	return *wrappedTFV == *derived.wrappedTFV && valueTranslation == derived.valueTranslation;
}

PiecewiseTFV::PiecewiseTFV() {
}

std::shared_ptr<TFV> PiecewiseTFV::clone() {
	std::shared_ptr<TFV> copy = std::make_shared<PiecewiseTFV>();
	copy->load(format());
	return copy;
}

std::string PiecewiseTFV::format() const {
	std::string ret = "";
	ret += formatString("PiecewiseTFV");
	for (auto segment : segments) {
		ret += tos(segment.first) + formatTMObject(*segment.second);
	}
	return ret;
}

void PiecewiseTFV::load(std::string formattedString) {
	auto items = split(formattedString, TextMarshallable::DELIMITER);
	for (int i = 1; i < items.size(); i += 2) {
		segments.push_back(std::make_pair(std::stof(items.at(i)), TFVFactory::create(items.at(i + 1))));
	}
}

nlohmann::json PiecewiseTFV::toJson() {
	nlohmann::json j;

	j["className"] = "PiecewiseTFV";

	nlohmann::json segmentsJson;
	for (std::pair<float, std::shared_ptr<TFV>> segment : segments) {
		segmentsJson.push_back(nlohmann::json{ {"beginTime", segment.first}, {"tfv", segment.second->toJson()} });
	}
	j["segments"] = segmentsJson;

	return j;
}

void PiecewiseTFV::load(const nlohmann::json& j) {
	segments.clear();
	if (j.contains("segments")) {
		for (const nlohmann::json& segmentJson : j.at("segments")) {
			std::pair<float, std::shared_ptr<TFV>> segment;
			segmentJson.at("beginTime").get_to(segment.first);
			segment.second = TFVFactory::create(segmentJson.at("tfv"));
			segments.push_back(segment);
		}
	}
}

float PiecewiseTFV::evaluate(float time) {
	int l = 0;
	int h = segments.size(); // Not n - 1
	while (l < h) {
		int mid = (l + h) / 2;
		if (time <= segments[mid].first) {
			h = mid;
		} else {
			l = mid + 1;
		}
	}

	int i;
	for (i = std::max(0, l - 1); i < segments.size(); i++) {
		if (i >= 0 && time < segments[i].first) {
			i--;
			break;
		}
	}

	auto it = segments.begin() + i;
	if (it == segments.end()) {
		return segments.back().second->evaluate(time - segments.back().first);
	} else {
		return it->second->evaluate(time - it->first);
	}
}

std::pair<float, int> PiecewiseTFV::piecewiseEvaluate(float time) {
	if (segments.size() == 0 || time < segments[0].first) {
		throw InvalidEvaluationDomainException();
	}

	int l = 0;
	int h = segments.size(); // Not n - 1
	while (l < h) {
		int mid = (l + h) / 2;
		if (time <= segments[mid].first) {
			h = mid;
		} else {
			l = mid + 1;
		}
	}

	int i;
	for (i = std::max(0, l - 1); i < segments.size(); i++) {
		if (i >= 0 && time < segments[i].first) {
			i--;
			break;
		}
	}

	auto it = segments.begin() + i;
	if (it == segments.end()) {
		return std::make_pair(segments.back().second->evaluate(time - segments.back().first), segments.size() - 1);
	} else {
		return std::make_pair(it->second->evaluate(time - it->first), it - segments.begin());
	}
}

void PiecewiseTFV::insertSegment(int index, std::pair<float, std::shared_ptr<TFV>> segment, float totalLifespan) {
	// Recalculate active times
	for (int i = index; i < segments.size(); i++) {
		segments[i].first += segment.first;
	}
	if (index != 0) {
		segment.first += segments[index - 1].first;
	} else {
		segment.first = 0;
	}
	segments.insert(segments.begin() + index, segment);
	recalculateMaxTimes(totalLifespan);
}

int PiecewiseTFV::insertSegment(std::pair<float, std::shared_ptr<TFV>> segment, float totalLifespan) {
	int l = 0;
	int h = segments.size();
	while (l < h) {
		int mid = (l + h) / 2;
		if (segment.first <= segments[mid].first) {
			h = mid;
		} else {
			l = mid + 1;
		}
	}

	int i;
	for (i = std::max(0, l - 1); i < segments.size(); i++) {
		if (i >= 0 && segment.first < segments[i].first) {
			i--;
			break;
		}
	}

	if (i == -1) {
		segments.insert(segments.begin(), segment);
		recalculateMaxTimes(totalLifespan);
		return 0;
	} else {
		segments.insert(segments.begin() + i, segment);
		recalculateMaxTimes(totalLifespan);
		return i;
	}
}

void PiecewiseTFV::removeSegment(int index, float totalLifespan) {
	auto erasedActiveTime = (segments.begin() + index)->first;
	segments.erase(segments.begin() + index);
	// Recalculate active times
	for (int i = index; i < segments.size(); i++) {
		segments[i].first -= erasedActiveTime;
	}
	recalculateMaxTimes(totalLifespan);
}

void PiecewiseTFV::changeSegment(int index, std::shared_ptr<TFV> newTFV, float totalLifespan) {
	segments[index].second = newTFV;
	recalculateMaxTimes(totalLifespan);
}

int PiecewiseTFV::changeSegmentStartTime(int segmentIndex, float newStartTime, float totalLifespan) {
	segments[segmentIndex].first = newStartTime;
	if (segmentIndex != 0 && newStartTime < segments[segmentIndex - 1].first) {
		int i = segmentIndex - 1;
		while (i != -1 && newStartTime < segments[i].first) {
			std::iter_swap(segments.begin() + i, segments.begin() + i + 1);
			i--;
		}
		return i + 1;
	} else if (segmentIndex != segments.size() - 1 && newStartTime > segments[segmentIndex + 1].first) {
		int i = segmentIndex + 1;
		while (i != segments.size() && newStartTime > segments[i].first) {
			std::iter_swap(segments.begin() + i, segments.begin() + i - 1);
			i++;
		}
		return i - 1;
	}
	recalculateMaxTimes(totalLifespan);
	return segmentIndex;
}

std::pair<float, std::shared_ptr<TFV>> PiecewiseTFV::getSegment(int index) {
	return segments[index];
}

int PiecewiseTFV::getSegmentsCount() {
	return segments.size();
}

void PiecewiseTFV::setMaxTime(float maxTime) {
	this->maxTime = maxTime;
	recalculateMaxTimes(maxTime);
}

bool PiecewiseTFV::operator==(const TFV& other) const {
	const PiecewiseTFV& derived = dynamic_cast<const PiecewiseTFV&>(other);
	return segments.size() == derived.segments.size() && std::equal(segments.begin(), segments.end(), derived.segments.begin());
}

void PiecewiseTFV::recalculateMaxTimes(float totalLifespan) {
	int index = 0;
	for (int i = 0; i < segments.size(); i++) {
		auto segment = segments[i];

		float nextTime = totalLifespan;
		if (i + 1 < segments.size()) {
			nextTime = segments[i + 1].first;
		}

		segment.second->setMaxTime(nextTime - segment.first);
		index++;
	}
}


std::shared_ptr<TFV> TFVFactory::create(std::string formattedString) {
	auto name = split(formattedString, TextMarshallable::DELIMITER)[0];
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
	else if (name == "PiecewiseTFV") {
		ptr = std::make_shared<PiecewiseTFV>();
	}
	ptr->load(formattedString);
	return std::move(ptr);
}

std::shared_ptr<TFV> TFVFactory::create(const nlohmann::json& j) {
	if (j.contains("className")) {
		std::string name;
		j.at("className").get_to(name);

		std::shared_ptr<TFV> ptr;
		if (name == "ConstantTFV") {
			ptr = std::make_shared<ConstantTFV>();
		} else if (name == "LinearTFV") {
			ptr = std::make_shared<LinearTFV>();
		} else if (name == "SineWaveTFV") {
			ptr = std::make_shared<SineWaveTFV>();
		} else if (name == "ConstantAccelerationDistanceTFV") {
			ptr = std::make_shared<ConstantAccelerationDistanceTFV>();
		} else if (name == "DampenedStartTFV") {
			ptr = std::make_shared<DampenedStartTFV>();
		} else if (name == "DampenedEndTFV") {
			ptr = std::make_shared<DampenedEndTFV>();
		} else if (name == "DoubleDampenedTFV") {
			ptr = std::make_shared<DoubleDampenedTFV>();
		} else if (name == "TranslationWrapperTFV") {
			ptr = std::make_shared<TranslationWrapperTFV>();
		} else if (name == "PiecewiseTFV") {
			ptr = std::make_shared<PiecewiseTFV>();
		}
		ptr->load(j);
		return std::move(ptr);
	} else {
		return std::make_shared<ConstantTFV>(0);
	}
}

CurrentAngleTFV::CurrentAngleTFV(entt::DefaultRegistry& registry, uint32_t from, uint32_t to) : registry(registry), from(from), to(to) {
	useEntityForToPos = true;
	assert(registry.has<PositionComponent>(from) && registry.has<PositionComponent>(to) && registry.has<HitboxComponent>(from) && registry.has<HitboxComponent>(to));
}

CurrentAngleTFV::CurrentAngleTFV(entt::DefaultRegistry& registry, uint32_t from, float toX, float toY) : registry(registry), from(from), toX(toX), toY(toY) {
	useEntityForToPos = false;
	assert(registry.has<PositionComponent>(from) && registry.has<HitboxComponent>(from));
}

std::shared_ptr<TFV> CurrentAngleTFV::clone() {
	return std::make_shared<CurrentAngleTFV>(registry, from, to);
}

float CurrentAngleTFV::evaluate(float time) {
	auto& fromPos = registry.get<PositionComponent>(from);
	auto& fromHitbox = registry.get<HitboxComponent>(from);

	if (useEntityForToPos) {
		auto& toPos = registry.get<PositionComponent>(to);
		auto& toHitbox = registry.get<HitboxComponent>(to);
		return std::atan2((toPos.getY() + toHitbox.getY()) - (fromPos.getY() + fromHitbox.getY()), (toPos.getX() + toHitbox.getX()) - (fromPos.getX() + fromHitbox.getX()));
	} else {
		return std::atan2(toY - (fromPos.getY() + fromHitbox.getY()), toX - (fromPos.getX() + fromHitbox.getX()));
	}
}

bool CurrentAngleTFV::operator==(const TFV& other) const {
	return true;
}
