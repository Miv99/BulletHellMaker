#include "TimeFunctionVariable.h"

std::shared_ptr<TFV> ConstantTFV::clone() {
	return std::make_shared<ConstantTFV>(value);
}

std::string ConstantTFV::format() const {
	return formatString("ConstantTFV") + tos(value);
}

void ConstantTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	value = std::stof(items[1]);
}

std::shared_ptr<TFV> LinearTFV::clone() {
	return std::make_shared<LinearTFV>(startValue, endValue, maxTime);
}

std::string LinearTFV::format() const {
	return formatString("LinearTFV") +  tos(startValue) +  tos(endValue) +  tos(maxTime);
}

void LinearTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	startValue = std::stof(items[1]);
	endValue = std::stof(items[2]);
	maxTime = std::stof(items[3]);
}

std::shared_ptr<TFV> SineWaveTFV::clone() {
	return std::make_shared<SineWaveTFV>(period, amplitude, valueShift, phaseShift);
}

std::string SineWaveTFV::format() const {
	return formatString("SineWaveTFV") +  tos(period) +  tos(amplitude) +  tos(valueShift) +  tos(phaseShift);
}

void SineWaveTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	period = std::stof(items[1]);
	amplitude = std::stof(items[2]);
	valueShift = std::stof(items[3]);
	phaseShift = std::stof(items[4]);
}

std::shared_ptr<TFV> ConstantAccelerationDistanceTFV::clone() {
	return std::make_shared<ConstantAccelerationDistanceTFV>(initialDistance, initialVelocity, acceleration);
}

std::string ConstantAccelerationDistanceTFV::format() const {
	return formatString("ConstantAccelerationDistanceTFV") + tos(initialDistance) + tos(initialVelocity) + tos(acceleration);
}

void ConstantAccelerationDistanceTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	initialDistance = std::stof(items[1]);
	initialVelocity = std::stof(items[2]);
	acceleration = std::stof(items[3]);
}

std::shared_ptr<TFV> DampenedStartTFV::clone() {
	std::shared_ptr<TFV> copy = std::make_shared<DampenedStartTFV>();
	copy->load(format());
	return copy;
}

std::string DampenedStartTFV::format() const {
	return formatString("DampenedStartTFV") + tos(a) + tos(startValue) + tos(dampeningFactor);
}

void DampenedStartTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	a = std::stof(items[1]);
	startValue = std::stof(items[2]);
	dampeningFactor = std::stoi(items[3]);
}

std::shared_ptr<TFV> DampenedEndTFV::clone() {
	std::shared_ptr<TFV> copy = std::make_shared<DampenedEndTFV>();
	copy->load(format());
	return copy;
}

std::string DampenedEndTFV::format() const {
	return formatString("DampenedEndTFV") + tos(a) + tos(endValue) + tos(maxTime) + tos(dampeningFactor);
}

void DampenedEndTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	a = std::stof(items[1]);
	endValue = std::stof(items[2]);
	maxTime = std::stof(items[3]);
	dampeningFactor = std::stoi(items[4]);
}

std::shared_ptr<TFV> DoubleDampenedTFV::clone() {
	return std::make_shared<DoubleDampenedTFV>(startValue, endValue, maxTime, dampeningFactor);
}

std::string DoubleDampenedTFV::format() const {
	return formatString("DoubleDampenedTFV") + tos(a) + tos(startValue) + tos(endValue) + tos(maxTime) + tos(dampeningFactor);
}

void DoubleDampenedTFV::load(std::string formattedString) {
	auto items = split(formattedString, DELIMITER);
	a = std::stof(items[1]);
	startValue = std::stof(items[2]);
	endValue = std::stof(items[3]);
	maxTime = std::stof(items[4]);
	dampeningFactor = std::stoi(items[5]);
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
	auto items = split(formattedString, DELIMITER);
	valueTranslation = std::stof(items[1]);
	wrappedTFV = TFVFactory::create(items[2]);
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
	auto items = split(formattedString, DELIMITER);
	for (int i = 1; i < items.size(); i += 2) {
		segments.push_back(std::make_pair(std::stof(items[i]), TFVFactory::create(items[i + 1])));
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

void PiecewiseTFV::insertSegment(std::pair<float, std::shared_ptr<TFV>> segment, float totalLifespan) {
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

	auto it = segments.begin() + i;
	segments.insert(it, segment);
	recalculateMaxTimes(totalLifespan);
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
	else if (name == "PiecewiseTFV") {
		ptr = std::make_shared<PiecewiseTFV>();
	}
	ptr->load(formattedString);
	return std::move(ptr);
}

std::shared_ptr<TFV> CurrentAngleTFV::clone() {
	return std::make_shared<CurrentAngleTFV>(registry, from, to);
}
